# Scene serialization

Save and load scenes in the shared course JSON format. Implemented in
[`src/serialization/SceneSerialization.{h,cpp}`](../OpenglGeometry/src/serialization/); the
format itself is decoded in [`plans/reference/format-spec.md`](../plans/reference/format-spec.md).

```cpp
namespace Serialization
{
    struct Result { bool ok; std::string message; };
    Result SaveScene(Scene& scene, const std::string& path);
    Result LoadScene(Scene& scene, const std::string& path);   // replaces the scene
}
```

`Result::message` is written to the status bar; failures also go to `Logger::Error`. The two
functions are **not** an `ISystem` — `SceneFileDialog.cpp`'s scene-aware overloads call them
directly from `GUISystem::Process`, where no ECS view is being iterated.

## The Save / Open dialog

`SceneFileDialog.cpp` — an editable **Path** field plus an in-app file browser built on
`std::filesystem` (no native OS dialog). The browser lists `..`, sub-directories as
`[ name ]`, and `*.json` files under the field's current directory; clicking a folder
navigates, clicking a file fills the path (the current file is highlighted via
`fs::equivalent`), double-clicking a file confirms immediately. Typing a valid directory into
the field snaps the browser to it. Paths are stored relative to the working directory when the
target sits under it, absolute otherwise (`SceneRelativePath`). The four `Draw*SceneDialog`
overloads each keep their own `static std::string path`; the browser state (`browseDir`) is a
single `static` shared across them — fine because only one dialog is open at a time.

## File shape

```json
{
  "points":   [ { "id": 1, "name": "Point 1", "position": { "x": .., "y": .., "z": .. } } ],
  "geometry": [ /* torus | chain | bezierC0 | bezierC2 | interpolatedC2
                  | bezierSurfaceC0 | bezierSurfaceC2 */ ]
}
```

- **One id namespace** across `points` and `geometry`; ids need only be unique within the file.
- `additionalProperties: false` everywhere — no `version`, no `camera`.
- `quaternion` is `{x,y,z,w}`; `Algebra::Quaternion` is `(w,x,y,z)` — the DTO maps **by name**.
- Surface `size` counts **control points** (`{u,v}`), not patches. `controlPoints` is
  **row-major, v rows of u**: index `(iu, iv)` is `iv * size.u + iu`.
- A cylinder has **no flag** — the wrapped edge's `controlPoints` repeat the ids of the
  opposite edge (one column/row for C0, three for C2).

## Export — `Scene → json`

Iterate `<IdComponent, ObjectTypeComponent>`:

| Type | Source |
| --- | --- |
| `Point` | `PositionComponent` |
| `torus` | `PositionComponent`, `RotationComponent` (identity if absent), `ScaleComponent`, `TorusGenerationComponent` — `samples = {radialSegments, tubularSegments}`, `smallRadius = tubeRadius`, `largeRadius = radius` |
| `chain` / `bezierC0` / `bezierC2` / `interpolatedC2` | `LineGenerationComponent::controlPoints`, in order, valid entities only |
| `bezierSurfaceC0` / `bezierSurfaceC2` | grid rebuilt from `bezierPatches` (`patch[i][j].controlPoints[x][y] == grid[i·stride+x][j·stride+y]`, stride 3 / 1); `size` from patch counts, `samples` from `samplesU/samplesV` |

Cylinder seams export for free: the aliased grid entities have the same `IdComponent::id`, so
the id simply repeats in the flat array.

`float` widens to `double` in nlohmann and `dump(2)` writes the full round-trippable form
(`3.69f → 3.690000057220459`), which the format expects.

## Import — `json → validate → Scene`

1. `json::parse`; a parse error returns `Result{false, ...}` with the scene untouched.
2. **Validate the DTO before mutating anything**: ids unique in the file; every `controlPoints`
   id exists in `points`; per surface `controlPoints.size() == size.u * size.v` and the axis
   sizes are a whole number of patches (C0: `(size-1) % 3 == 0 && size >= 4`; C2: `size >= 4`).
3. `Scene::Clear()` — destroys everything with an `IdComponent` (and its virtual children).
   Camera, grid, cursor and the selection marker have no `IdComponent` and survive.
4. Create every point with `Archetypes::CreatePoint`; build `unordered_map<fileId, Entity>`.
   File ids are **discarded** — fresh `IdManager` ids are allocated.
5. Create geometry, resolving `controlPoints` through the map (a repeated id resolves to the
   same `Entity`, which is how cylinder seams are reconstructed):
   - curves → `Archetypes::CreatePolyline / CreateBezierC0 / CreateBezierC2 / CreateInterpolatedBezier`
   - torus → `AddShapeToEntity` + `AddTorusToEntity`, then overwrite the DTO fields
   - surfaces → `Archetypes::CreateBezierSurfaceFromControlGrid(scene, grid, samplesU, samplesV, isC2)`
6. Unknown `objectType` is skipped with a `Logger::Warning`, not rejected.

## Fixture

[`scenes/example_scene.json`](../scenes/example_scene.json) is the course's 170-point example
(1 torus, bezierC0, bezierC2, interpolatedC2, 2× bezierSurfaceC0 — one a v-wrapped cylinder —
and 1 bezierSurfaceC2, also a v-wrapped cylinder). Verified:

- loads to 170 points + 7 objects, all rendering correctly (the C2 cylinder is a smooth closed
  tube; the wraps are reconstructed from the repeated ids);
- load → save produces a **semantically identical** file (point count, positions, each object's
  type / control-point count / `size` / `samples`, and both surface wraps);
- the saved output **validates against the course `schema.json`** (draft 2020-12).

The course reference (`schema.json`, `readme.md`, `example_scene.json`) is kept under
`plans/reference/format/` (local-only, like the rest of `plans/`).

## Known gaps

- **File picker** — the dialog is a plain path text field; there is no native Open/Save dialog
  (`comdlg32` is not linked).
- **Interop** — the torus `samples` u/v direction and the handedness convention are the
  natural reading and match the course example, but have not been exchanged with another
  student's application.
