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

---

## `ShapeInspectorSystem`

[`ShapeInspectorSystem.h`](../../OpenglGeometry/src/systems/gui/ShapeInspectorSystem.h) ·
[`.cpp`](../../OpenglGeometry/src/systems/gui/ShapeInspectorSystem.cpp)

Draws the *Selected Shapes Properties* panel. It inherits from **both**
[`ComponentFunctionRegistry<>`](../ecs/component-function-registry.md) and `ISystem`:

```cpp
ShapeInspectorSystem::ShapeInspectorSystem(Ref<Scene> scene)
    : m_Scene{ scene }
{
    Bind<PositionComponent>(&ShapeInspectorSystem::PositionInspect);
    Bind<RotationComponent>(&ShapeInspectorSystem::RotationInspect);
    Bind<ScaleComponent>(&ShapeInspectorSystem::ScaleInspect);
    Bind<LineGenerationComponent>(&ShapeInspectorSystem::LineInspect);
    Bind<IsParentOfVirtualEntitiesComponent>(&ShapeInspectorSystem::VirtualInspect);
    Bind<TorusGenerationComponent>(&ShapeInspectorSystem::TorusInspect);
}

void ShapeInspectorSystem::Process()
{
    ImGui::Begin("Selected Shapes Properties##Selected Shapes Properties");
    auto selectedShapes = m_Scene->GetAllEntitiesWith<IsSelectedTag>();

    if (selectedShapes.empty())
        ImGui::Text("No shapes selected.");

    for (Entity entity : selectedShapes)
    {
        ImGui::Text("Properties of %s", entity.GetComponent<NameComponent>().name.c_str());
        this->PerformFunctions(entity);      // ← runs every bound handler this entity has
    }

    ImGui::End();
}
```

Each selected entity contributes exactly the editors for the components it actually carries.
Adding a new editable component is one `Bind` plus one handler — see
[how-to/add-an-inspector-field.md](../how-to/add-an-inspector-field.md).

### The handlers

| Handler | Component | Widget | Side effect |
| --- | --- | --- | --- |
| `PositionInspect` | `PositionComponent` | `DragFloat3` | Assigns through `Observable` → `ObserverChangedState` |
| `RotationInspect` | `RotationComponent` | `DragFloat3` on `rotation.x/y/z` | Mutates in place |
| `ScaleInspect` | `ScaleComponent` | `DragFloat3` | Mutates in place |
| `TorusInspect` | `TorusGenerationComponent` | `DragFloat` ×2, `GUI::DragUInt` ×2 | Adds `IsDirtyTag` on change |
| `LineInspect` | `LineGenerationComponent` | Read-only text | none |
| `VirtualInspect` | `IsParentOfVirtualEntitiesComponent` | `Checkbox` | Toggles `IsInvisibleTag` on children |

`PositionInspect` shows the required pattern for observable values — copy out, edit the copy,
assign back:

```cpp
Algebra::Vector4 tmpPosition = entity.GetComponent<PositionComponent>().position;
auto& position = entity.GetComponent<PositionComponent>().position;
if (ImGui::DragFloat3(GUI::GenerateLabel(entity, "Position").c_str(), &tmpPosition.x, 0.1f))
    position = tmpPosition;
```

`TorusInspect` shows the required pattern for generation parameters — mark dirty on every
widget that returns `true`:

```cpp
if (ImGui::DragFloat(GUI::GenerateLabel(entity, "Radius").c_str(), &torusComponent.radius, 0.1f))
    entity.AddTag<IsDirtyTag>();
```

Segment counts are clamped to `[3, 64]` by `GUI::DragUInt`.

`VirtualInspect` renders one checkbox per virtual child but generates the label from the
*parent* entity, so with more than one child the labels collide and only one checkbox is
functional — see [gotchas](../gotchas.md).

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
    auto cursorPosition = m_Cursor.GetComponent<PositionComponent>().position;

    if (ImGui::MenuItem("Create Torus##Creation menu"))
        Archetypes::CreateTorus(m_Scene.get(), cursorPosition);

    if (ImGui::MenuItem("Create Point##Creation menu"))
        Archetypes::CreatePoint(m_Scene.get(), cursorPosition);

    if (ImGui::MenuItem("Create Polyline##Creation menu"))
    {
        auto selectedPoints = GetSelectedPoints();
        Archetypes::CreatePolyline(m_Scene.get(), selectedPoints.begin(), selectedPoints.end());
    }
    // ... Bezier C0, Bezier C2, Interpolated Bezier — same shape
}
```

`GetSelectedPoints` materialises the view into a `std::vector` before creating anything,
because the archetypes mutate the registry (adding components to the selected points):

```cpp
std::vector<Entity> ShapeCreation::GetSelectedPoints()
{
    auto pointsView = m_Scene->GetAllEntitiesWith<IsSelectedTag, NotificationComponent>();
    return std::vector<Entity>(pointsView.begin(), pointsView.end());
}
```

`<IsSelectedTag, NotificationComponent>` is the idiom for "selected points" —
`NotificationComponent` is what marks an entity as usable as a control point.

Note that *Create Bézier Surface* is not in the menu, even though
`Archetypes::CreateBezierSurface` exists and works. Adding it would require a parameter
dialog for `BezierSurfaceCreationParameters` (patch counts, size, cylinder flag).

### Adding a popup

1. Implement `IPopup` under `src/UI/popups/`.
2. `ShouldOpen()` returns your trigger, e.g.
   `ImGui::IsKeyChordPressed(ImGuiMod_Shift | ImGuiKey_A)`.
3. Push it into `m_RegisteredPopups` in `PopupSystem`'s constructor.
4. Add the new `.cpp` to the `.vcxproj`
   ([how-to](../how-to/add-a-file-to-the-build.md)).
