# UI

Two things live under [`src/UI/`](../OpenglGeometry/src/UI/): reusable ImGui helpers, and
popup implementations. The *systems* that call them are documented in
[systems/gui-systems.md](systems/gui-systems.md).

| File | Contents |
| --- | --- |
| `GUI.h` | Umbrella header (currently just includes `ShapeList.h`) |
| `ShapeList.h` | `GUI::DisplayShapeList` — the shape list panel |
| `Utils.h` | `GUI::GenerateLabel`, `GUI::DragUInt` |
| `popups/ShapeCreation.{h,cpp}` | The Shift + A creation menu |

Everything in `namespace GUI` is `inline` and header-only.

---

## Panels

The application has two ImGui windows plus (in debug builds) the ImGui demo window:

| Window | Drawn by |
| --- | --- |
| *Shape List* | `GUI::DisplayShapeList`, called from `GUISystem::Process` |
| *Selected Shapes Properties* | `ShapeInspectorSystem::Process` |
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
