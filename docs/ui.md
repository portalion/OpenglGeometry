# UI

Two things live under [`src/UI/`](../OpenglGeometry/src/UI/): reusable ImGui helpers, and
popup implementations. The *systems* that call them are documented in
[systems/gui-systems.md](systems/gui-systems.md).

| File | Contents |
| --- | --- |
| `GUI.h` | Umbrella header (currently just includes `ShapeList.h`) |
| `ShapeList.h` | `GUI::DisplayShapeList` — the shape list panel |
| `Utils.h` | `GUI::GenerateLabel`, `GUI::DragUInt` |
| `ViewportPicking.{h,cpp}` | `GUI::HandleViewportPicking` — click / box selection of points in the 3D viewport |
| `CursorControl.{h,cpp}` | `GUI::HandleCursorPlacement` — right-click to move the 3D cursor |
| `ViewportMath.{h,cpp}` | `ActiveViewportCamera`, `ProjectToViewport`, `ViewportRayDirection` — shared world↔screen maths |
| `CursorPanel.{h,cpp}` | `GUI::DrawCursorPanel` — the *3D Cursor* panel |
| `popups/ShapeCreation.{h,cpp}` | The Shift + A creation menu |

Small pure-function helpers in `namespace GUI` are `inline` and header-only; the panel and
picking helpers with real logic have a `.cpp` (listed in `OpenglGeometry/CMakeLists.txt`).

---

## Panels

The application's dockable ImGui windows, all driven from `GUISystem::Process` (plus the demo
window in debug builds):

| Window | Drawn by |
| --- | --- |
| *Shape List* | `GUI::ShapeList::Display` |
| *Selected Shapes Properties* | `ShapeInspectorRegistry::Display` |
| *3D Cursor* | `GUI::DrawCursorPanel` |
| *Dear ImGui Demo* | `App::Run`, `#ifdef _DEBUG` only |

Docking is enabled (`ImGuiConfigFlags_DockingEnable` in `InitImgui`), so the user can
rearrange them; the layout persists to `imgui.ini`, which is gitignored.

The GL viewport is narrowed by `Globals::rightInterfaceWidth` (400 px) so the panels do not
overlap the 3D scene — see `Window::HandleResize`.

---

## `GUI::DisplayShapeList`

[`src/UI/ShapeList.h`](../OpenglGeometry/src/UI/ShapeList.h)

```cpp
inline void DisplayShapeList(Ref<Scene> scene)
{
    ImGui::Begin("Shape List##Shape List");

    auto shapes = scene->GetAllEntitiesWith<NameComponent>();

    if (shapes.empty()) { ImGui::Text("No shapes available.##Shape List"); ImGui::End(); return; }

    if (ImGui::Button("Select All##Shape List"))       /* add IsSelectedTag to all */
    if (ImGui::Button("Deselect All##Shape List"))     /* remove IsSelectedTag from all */
    if (ImGui::Button("Remove All Selected##Shape List")) /* add ToBeDestroyedTag to selected */

    for (Entity entity : shapes)
    {
        bool isSelected = entity.HasComponent<IsSelectedTag>();
        if (ImGui::Selectable(GenerateLabel(entity, entity.GetComponent<NameComponent>().name).c_str(),
                              isSelected))
        {
            if (isSelected) entity.RemoveTag<IsSelectedTag>();
            else            entity.AddTag<IsSelectedTag>();
        }
    }

    ImGui::End();
}
```

Two things determine what appears here:

- **`NameComponent` is the filter.** An entity is listed if and only if it has one. That is
  why `Archetypes::AddShapeToEntity` adds it, and why the grid and Bézier patches do not have
  it.
- **Deletion is deferred.** *Remove All Selected* only tags; `RemovalSystem` does the work at
  the start of the next frame.

Rows are plain `Selectable`s — no multi-select modifiers, no ordering, no filtering. The
loop mutates tags while iterating the view; `IsSelectedTag` is not part of the view's filter
(`<NameComponent>`), so this is safe.

---

## `GUI::GenerateLabel`

[`src/UI/Utils.h`](../OpenglGeometry/src/UI/Utils.h)

```cpp
inline std::string GenerateLabel(Entity entity, std::string labelWithoutId)
{
    return labelWithoutId + "##" + entity.GetID();
}
```

ImGui identifies widgets by their label string. Two entities both showing a "Position"
`DragFloat3` would share an ID and interfere with each other — dragging one would move both.
The `##` suffix is hidden from the user but makes the ID unique.

**Every widget rendered per-entity must use this.** The convention throughout the inspector:

```cpp
ImGui::DragFloat3(GUI::GenerateLabel(entity, "Position").c_str(), &tmpPosition.x, 0.1f);
```

`Entity::GetID()` returns the raw EnTT handle index as a string — unique among live entities.
(It is *not* `IdComponent::id`, which is the display number in the shape name.)

Widgets that are not per-entity hard-code their own suffix instead, e.g.
`"Select All##Shape List"`.

> `VirtualInspect` renders one checkbox per virtual child but generates the label from the
> *parent*, so with multiple children the IDs collide. See [gotchas](gotchas.md).

## `GUI::DragUInt`

```cpp
inline bool DragUInt(const char* label, uint32_t* value,
                     float speed = 1.0f, uint32_t min = 0, uint32_t max = 0)
{
    return ImGui::DragScalar(label, ImGuiDataType_U32, value, speed, &min, &max);
}
```

ImGui has `DragInt` but no unsigned variant; this wraps `DragScalar`. Used for the torus
segment counts with `min = 3, max = 64`.

Note `min`/`max` are taken by value and their addresses passed to ImGui — fine because
`DragScalar` reads them synchronously.

---

## `GUI::HandleViewportPicking`

```cpp
struct ViewportPickingState { bool dragging; ImVec2 dragStart; };

void HandleViewportPicking(Ref<Scene> scene, const Dockspace& dockspace, ViewportPickingState& state);
```

Called once per frame from `GUISystem::Process`; the `state` is a `GUISystem` member so the
drag survives between frames. Behaviour is described in
[systems/gui-systems.md](systems/gui-systems.md#viewport-point-picking) — click picks the
nearest point, drag box-selects, Shift/Ctrl make it additive, and it only tags
`ObjectType::Point` entities.

`Dockspace::TryGetCentralNodeScreenRect` gives it the viewport rectangle in the same ImGui
coordinate space as `ImGui::GetMousePos()` (unlike `TryGetCentralNodeRect`, which is
framebuffer pixels for `glViewport`).

---

## `GUI::HandleCursorPlacement`

```cpp
struct CursorPlacementState { bool tracking; ImVec2 pressPos; };

void HandleCursorPlacement(Ref<Scene> scene, const Dockspace& dockspace,
    const CursorState& cursorState, CursorPlacementState& state);
```

Also called every frame from `GUISystem::Process`. A right-click tap in the viewport moves the
`CursorTag` entity; a right-drag is left alone for the camera. `cursorState.snapToNearest`
(set by the *3D Cursor* panel) chooses between snapping to the closest object and dropping onto
the camera-facing plane through the cursor's current position. See
[gui-systems.md](systems/gui-systems.md#viewport-cursor-placement).

## `ViewportMath`

```cpp
struct ViewportCamera { Matrix4 view, projection; Vector4 position, right, up, forward; };

std::optional<ViewportCamera> ActiveViewportCamera(Ref<Scene> scene);
bool ProjectToViewport(const ViewportCamera&, const Vector4& world,
    const ImVec2& rectMin, const ImVec2& rectMax, ImVec2& outScreen);   // false if behind camera
Vector4 ViewportRayDirection(const ViewportCamera&, const ImVec2& screen,
    const ImVec2& rectMin, const ImVec2& rectMax);                      // normalised, from position
```

Shared world↔screen maths for picking and cursor placement. `ProjectToViewport` reproduces the
`projection * view` transform the vertex shaders do; `ViewportRayDirection` builds a pinhole ray
from the camera basis and the FOV encoded in `projection[0][0]` / `[1][1]`. Both use the camera
as it was at the end of the previous frame (see the picking note above).

## `GUI::DrawCursorPanel`

```cpp
void DrawCursorPanel(UiState& state);                                        // sandbox, fixture only
void DrawCursorPanel(Ref<Scene> scene, UiState& state, const Dockspace&);     // live, scene-synced
```

The live overload two-way-binds `UiState::cursor` to the scene's cursor entity: entity → panel
at the top (`world`, projected `screenX/Y`, selection `centre` + count), panel → entity at the
bottom if `world` changed. The one-arg overload is the pre-existing fixture version the
UI sandbox (`--ui-sandbox`) still uses.

---

## `IPopup`

[`src/interfaces/IPopup.h`](../OpenglGeometry/src/interfaces/IPopup.h)

```cpp
class IPopup
{
public:
    virtual const char* Name()  = 0;   // ImGui popup identifier
    virtual bool ShouldOpen()   = 0;   // polled every frame
    virtual void Display()      = 0;   // body, called between Begin/EndPopup
};
```

Implementations are registered with [`PopupSystem`](systems/gui-systems.md#popupsystem),
which handles `OpenPopup` / `BeginPopup` / `EndPopup` so `Display()` only writes the contents.

`IPopup` has no virtual destructor although it is held via `Ref<IPopup>`; `shared_ptr`'s
type-erased deleter makes this work in practice.

### `ShapeCreation`

The only popup today — the **Shift + A** menu, documented with its systems in
[systems/gui-systems.md](systems/gui-systems.md#shapecreation).

---

## Conventions for new UI

- **Always** wrap per-entity widget labels in `GUI::GenerateLabel`.
- Prefer adding a handler to `ShapeInspectorSystem` over writing a new panel — it gets
  per-component dispatch for free
  ([how-to](how-to/add-an-inspector-field.md)).
- After mutating a value that affects generated geometry, set `IsDirtyTag`.
- After mutating a value other entities depend on, assign through the `Observable` rather
  than mutating in place.
- Deletion goes through `ToBeDestroyedTag`, never `Scene::DestroyEntity`.
- Header-only helpers go in `namespace GUI` under `src/UI/`; add the header to `GUI.h` if it
  should be part of the umbrella include.
