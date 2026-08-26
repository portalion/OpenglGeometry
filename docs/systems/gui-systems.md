# GUI systems

Three systems under [`src/systems/gui/`](../../OpenglGeometry/src/systems/gui/) draw the
interface. They run in positions 3–5 of the pipeline, after the scene has been cleaned up and
before meshes are regenerated — so an edit made this frame is reflected in the geometry drawn
this frame.

The reusable ImGui helpers they call live in [`src/UI/`](../ui.md).

---

## `GUISystem`

[`GUISystem.h`](../../OpenglGeometry/src/systems/gui/GUISystem.h) ·
[`.cpp`](../../OpenglGeometry/src/systems/gui/GUISystem.cpp)

Owns the 3D **cursor** entity and draws the *Shape List* panel.

```cpp
GUISystem::GUISystem(Ref<Scene> scene)
    : m_Scene(scene)
{
    m_Cursor = m_Scene->CreateEntity();
    Archetypes::AddShapeToEntity(m_Cursor, "Cursor");
    m_Cursor.AddComponent<PositionComponent>();
    auto& meshComponent = m_Cursor.AddComponent<MeshComponent>();
    meshComponent.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Cursor);
    meshComponent.shaderTypes.push_back(AvailableShaders::Default);
    meshComponent.renderingMode = RenderingMode::Lines;
}

void GUISystem::Process()
{
    if (!m_Cursor.HasComponent<PositionComponent>())
        m_Cursor.AddComponent<PositionComponent>();

    GUI::DisplayShapeList(m_Scene);
}
```

The cursor is a three-segment axis gizmo (`StaticMeshType::Cursor`) marking where new shapes
are created. It has a `NameComponent`, so it appears in the shape list and its position can
be edited in the inspector like any other shape. The re-add guard in `Process()` restores the
position component if the user deletes it via some other path.

> `ShapeCreation` creates a *second*, independent cursor entity with the same setup, and it
> is that one whose position is used when creating shapes. Two "Cursor" rows therefore appear
> in the shape list. See [gotchas](../gotchas.md).

The panel itself is [`GUI::DisplayShapeList`](../ui.md#displayshapelist) in
`src/UI/ShapeList.h`.

### Viewport point picking

At the end of `Process()`, `GUISystem` calls
[`GUI::HandleViewportPicking`](../ui.md#handleviewportpicking)
(`src/ui/ViewportPicking.{h,cpp}`), passing its `Dockspace` and a small persistent
`ViewportPickingState` member:

- **Left click** in the empty central (3D) area picks the nearest point within ~12 px
  (screen-space distance, camera depth breaks ties). A click on nothing clears the selection.
- **Left drag** rubber-bands a box (drawn on the ImGui foreground draw list, clamped to the
  viewport) and selects every point whose projected position lands inside it on release.
- **Shift / Ctrl** make either mode additive — a box adds, a click toggles the hit point.

It only ever touches entities with `ObjectTypeComponent == Point`. Points are projected on the
CPU with the active `CameraComponent`'s `viewMatrix` / `projectionMatrix`; because that view
matrix is written later in the frame by `RenderingSystem`, picking runs against the **previous
frame's** camera pose — invisible in practice. Camera orbit/pan/zoom use the right/middle
buttons, so left-drag selection never fights the camera.

### Viewport cursor placement

`GUISystem` also calls [`GUI::HandleCursorPlacement`](../ui.md#handlecursorplacement)
(`src/ui/CursorControl.{h,cpp}`) with a `CursorPlacementState` member:

- **Right click** (a tap — a right *drag* is a camera orbit, cancelled once travel passes
  `io.MouseDragThreshold`) in the viewport moves the scene's cursor entity
  (`CursorTag` + `PositionComponent`).
- **Keep depth** (default): the cursor lands on the camera-facing plane through its current
  position, so it keeps its distance from the eye.
- **Snap to nearest object**: it jumps to the closest object's position within 16 px, falling
  back to keep-depth when nothing is close.

The mode is the *On right-click* segmented control in the **3D Cursor** panel. That panel
([`GUI::DrawCursorPanel`](../ui.md#drawcursorpanel), `src/ui/CursorPanel.cpp`) is now wired to
the same entity: each frame it copies the entity's position into `UiState::cursor.world`,
projects it to the read-only *Screen X/Y* fields, fills *Centre* / the object count from the
live `IsSelectedTag` selection, and writes `world` back to the entity if the *World* row or the
*Cursor → centre* / *Cursor → origin* buttons changed it. The ray maths for both placement and
the *Screen X/Y* projection live in [`ViewportMath`](../ui.md#viewportmath).

`ShapeCreation` reads the same cursor via `Archetypes::GetCursorPosition`, so newly created
shapes appear wherever the cursor was last placed.

---

## The inspector (*Selected Shapes Properties*)

`GUISystem` renders the panel with [`GUI::DrawInspector`](../../OpenglGeometry/src/ui/Inspector.cpp)
— the panel that used to be sandbox-only. `GUISystem::Process` mirrors the scene into
`m_UiState` each frame (`SyncInspectorState`), calls `DrawInspector`, then pushes edits back
(`WriteBackInspectorState`):

- `m_UiState.objects` ← every `<IdComponent, NameComponent, ObjectTypeComponent>` entity, with
  `selected` from `IsSelectedTag`.
- On a **single** selection: `m_UiState.transform` ← `PositionComponent` / `RotationComponent`
  (quaternion → Euler degrees via `GUI::QuaternionToEulerDegrees`) / `ScaleComponent`;
  `m_UiState.torus` ← `TorusGenerationComponent`. A snapshot is kept; write-back only touches a
  component whose mirrored value actually changed (torus changes also add `IsDirtyTag`).
- `m_UiState.cursor.selectionCentre` ← `GUI::SelectionCentre`.

`m_UiState.curve` / `m_UiState.surface` are not synced yet, so the CURVE / SURFACE sections and
the old `LineInspect` / `VirtualInspect` editors are not shown. `ShapeInspectorRegistry` is the
former implementation and is now unused.

### Transform the selection

With ≥ 1 object selected, `DrawTransformSelectionBlock` (in `Inspector.cpp`) shows a relative
*Move / Rotate (deg) / Scale* delta plus a **pivot** toggle, applied on *Apply* through
`InspectorCallbacks::applySelectionTransform` → `GUI::ApplySelectionTransform`
(`ui/SceneActions.h`). The toggle is the same `UiState::pivot` the toolbar shows:

| Pivot (`PivotMode`) | `TransformSpace` | Behaviour |
| --- | --- | --- |
| `Origin` ("Own origin") | `Local` | each object about its own position, in its own frame (`rotation * delta`, intrinsic); Move is along local axes |
| `Cursor` ("3D cursor") | `Cursor` | world axes, about the shared 3D-cursor pivot (`delta * rotation`, extrinsic) |
| `Centre` | `Centre` | world axes, about the mean of the selected positions |

`ApplySelectionTransform` writes `PositionComponent` (through the `Observable`, so dependent
curves rebuild) / `RotationComponent` / `ScaleComponent` directly. Non-uniform scale of rotated
objects is applied per-component and introduces no shear — an approximation, matching the
`T * R * S` model matrix with no accumulated matrix.

---

## `PopupSystem`

[`PopupSystem.h`](../../OpenglGeometry/src/systems/gui/PopupSystem.h) ·
[`.cpp`](../../OpenglGeometry/src/systems/gui/PopupSystem.cpp)

A tiny dispatcher for modal popups implementing [`IPopup`](../ui.md#ipopup):

```cpp
class IPopup
{
public:
    virtual const char* Name()   = 0;   // ImGui popup id
    virtual bool ShouldOpen()    = 0;   // polled every frame; usually a key chord
    virtual void Display()       = 0;   // popup body
};
```

```cpp
void PopupSystem::Process()
{
    for (auto& popup : m_RegisteredPopups)
        if (popup->ShouldOpen())
        {
            ImGui::OpenPopup(popup->Name());
            m_OpenedPopup = popup;
            break;                       // at most one popup opens per frame
        }

    if (m_OpenedPopup != nullptr && ImGui::BeginPopup(m_OpenedPopup->Name()))
    {
        m_OpenedPopup->Display();
        ImGui::EndPopup();
    }
    else
    {
        m_OpenedPopup = nullptr;         // popup closed itself → forget it
    }
}
```

Popups are registered in the constructor:

```cpp
PopupSystem::PopupSystem(Ref<Scene> scene)
{
    m_RegisteredPopups.push_back(CreateRef<ShapeCreation>(scene));
}
```

### `ShapeCreation`

[`src/UI/popups/ShapeCreation.cpp`](../../OpenglGeometry/src/UI/popups/ShapeCreation.cpp) —
the **Shift + A** menu. Each menu item calls the matching archetype factory:

```cpp
void ShapeCreation::Display()
{
    if (ImGui::MenuItem("Create Torus##Creation menu"))
        GUI::CreateShape(m_Scene, ObjectType::Torus);

    if (ImGui::MenuItem("Create Point##Creation menu"))
        GUI::CreateShape(m_Scene, ObjectType::Point);

    if (ImGui::MenuItem("Create Polyline##Creation menu"))
        GUI::CreateShape(m_Scene, ObjectType::Chain);
    // ... Bezier C0, Bezier C2, Interpolated Bezier — same shape
}
```

`GUI::CreateShape` (`ui/SceneActions.h`) is the single dispatch point: it reads the 3D cursor,
and for curve types materialises the selected control points into a `std::vector` via
`GUI::GetSelectedControlPoints` before creating anything (the archetypes mutate the registry,
adding components to the selected points):

```cpp
std::vector<Entity> GUI::GetSelectedControlPoints(Ref<Scene> scene)
{
    auto view = scene->GetAllEntitiesWith<IsSelectedTag, NotificationComponent>();
    return std::vector<Entity>(view.begin(), view.end());
}
```

`<IsSelectedTag, NotificationComponent>` is the idiom for "selected points" —
`NotificationComponent` is what marks an entity as usable as a control point.

The menu-bar **Create** menu (`DrawCreateMenuItems(Ref<Scene>)` in `MenuItems.h`) goes through
the same `GUI::CreateShape`, so it now creates real entities too. Its *Bézier surface...* item
opens a parameter dialog (`BezierSurfaceDialog.cpp`) that calls `Archetypes::CreateBezierSurface`
with a `BezierSurfaceCreationParameters` built from the draft (patch counts, size, cylinder flag).

### Viewport context menu

`ContextMenu.h` — `GUI::DrawViewportContextMenu`, called from `GUISystem::Process`. **Shift +
right-click** in the central viewport opens an ImGui popup with the *Create / Edit / Select /
View* submenus, reusing the same `DrawCreateMenuItems` / `DrawEditMenuItems` /
`DrawSelectMenuItems` / `DrawViewMenuItems` helpers as the menu bar. Plain right-click still
places the 3D cursor — `HandleCursorPlacement` bails when `io.KeyShift` is held so the two
don't fight.

### Adding a popup

1. Implement `IPopup` under `src/UI/popups/`.
2. `ShouldOpen()` returns your trigger, e.g.
   `ImGui::IsKeyChordPressed(ImGuiMod_Shift | ImGuiKey_A)`.
3. Push it into `m_RegisteredPopups` in `PopupSystem`'s constructor.
4. Add the new `.cpp` to the `add_executable` list in `OpenglGeometry/CMakeLists.txt`
   ([how-to](../how-to/add-a-file-to-the-build.md)).
