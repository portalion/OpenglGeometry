# Surface intersections

Intersection curves between two parametric surfaces — torus, Bézier surface C0, Bézier
surface C2, in any combination, including a surface against itself.

| File | What it holds |
| --- | --- |
| [`geometry/ParametricSurface.h`](../../OpenglGeometry/src/geometry/ParametricSurface.h) | `IParametricSurface` and the torus / Bézier / point implementations |
| [`geometry/ParametricSurfaceFactory.h`](../../OpenglGeometry/src/geometry/ParametricSurfaceFactory.h) | `MakeParametricSurface(Entity)` — reads the scene components |
| [`geometry/IntersectionFinder.h`](../../OpenglGeometry/src/geometry/IntersectionFinder.h) | CG seed, 4×4 Newton march, `FindIntersections` |
| [`geometry/IntersectionSplit.h`](../../OpenglGeometry/src/geometry/IntersectionSplit.h) | `SplitSelfCrossings` — cuts a figure-eight into simple closed loops |
| [`geometry/TrimMask.h`](../../OpenglGeometry/src/geometry/TrimMask.h) | parameter-space flood fill that turns a closed curve into a two-region trim mask |

## The finder

```cpp
namespace Geometry
{
    struct IntersectionSettings
    {
        float stepLength = 0.05f;   // arc length between marched points
        float precision  = 1e-4f;   // ‖P − Q‖² accepted as "on the curve"
        bool  useCursor  = false;
        Algebra::Vector4 cursorPosition;
    };

    struct IntersectionData
    {
        std::vector<Algebra::Vector4> points;        // world-space polyline
        std::vector<Algebra::Vector4> paramsP;       // (u, v) on entityP
        std::vector<Algebra::Vector4> paramsQ;       // (s, t) on entityQ
        std::vector<uint32_t>         componentEnds; // one-past-last index per component
        bool closed = false;                         // every component closed
    };

    // entityQ invalid  => self-intersection of entityP.
    // Empty `points` => no intersection.
    IntersectionData FindIntersections(Entity, Entity, const IntersectionSettings&);
}
```

### Algorithm

1. **Seeds.** `CollectSeeds` samples a 24⁴ grid on the two parameter squares, keeps the nearest
   `Q` for every `P` sample, sorts by gap, and refines the best ~120 into up to 24 **distinct**
   seeds (world-space points more than a hair apart). Two-surface refinement is **`MinimizeGapCG`**
   (Fletcher–Reeves conjugate gradient, Armijo line search on `‖P − Q‖²`) then **`PolishSeed`**
   (least-norm Gauss–Newton on `P − Q = 0`). A **self-intersection** cannot use either — the
   global minimum of `‖P(x) − P(y)‖²` is the trivial `x == y` diagonal, which both slide onto —
   so it uses **`PolishSelfSeed`**: the marcher's own 4×4 correction with a zero arc step (three
   rows `P(x) − P(y) = 0`, one row pinning the point along the frozen crossing tangent
   `n(x) × n(y)`, which degenerates on the diagonal and pushes away from it). A seed is kept
   only if its polished gap reaches the evaluation floor (`kSeedGapTolerance`, 1e-8 on the
   squared gap) — a near-miss stalls far above that and is dropped, so it no longer produces a
   phantom curve — and, for a self-intersection, only if it stays `minSeparation` off the
   diagonal, measured by `ParamSeparation` (the `(u,v)` gap folded across any wrapped axis, so a
   seed straddling a seam is not mistaken for a second sheet). Cursor mode (two-surface only)
   projects the 3D cursor onto each surface and refines that one pair.
2. **March.** `NewtonCorrect` is one 4×4 step: three rows `P − Q = 0`, one row fixing the
   arc-length advance `(midpoint − anchor)·tangent = ±stepLength`, solved by Gaussian
   elimination with partial pivoting (`SolveLinear4`). The tangent is `n_P × n_Q`, sign-locked
   to the previous accepted step (`n_P × n_Q` flips whenever either surface normal does; without
   the lock the march doubles back on itself at a self-intersection, where the two sheets meet
   at a shallow angle). A step that stalls or oversteps (`|candidate − anchor|² > 9·stepLength²`,
   kept separate from the residual `precision`) is halved up to 6×; if it still fails — a
   tangency, or the curve left the domain — the march in that direction stops.
3. **Both ways.** `TraceIntersection` marches `+stepLength` then `−stepLength` from the seed and
   joins the halves. A march that returns within one step of its start closes the loop.
4. **Every component.** `FindIntersectionComponents` traces every seed from step 1 and keeps a
   trace unless it wandered (arc length > 16× its own bounding-box extent — a marcher that
   stalled at a tangency and looped) or it mostly retraces a component already kept (the
   swapped `P ↔ Q` view of a self-crossing, or the same loop from a nearby seed). Each kept
   trace appends to `points` / `paramsP` / `paramsQ`, and its end index is pushed to
   **`componentEnds`** so the mesh, the parameter-space panel and the trim mask can break the
   polyline between components.

`paramsP` / `paramsQ` fall out of the march for free. `ProjectOntoSurface` /
`FillParametersFromPoints` remain in the file as standalone helpers for a caller that only has
world points.

**Multiple components.** Two surfaces meeting in several loops, or a surface that folds through
itself, all come out — e.g. `B-Patches 1` in the course scene self-intersects in two roughly
perpendicular loops (a tube passing through itself), and two near-coincident tori meet in two
circles. A curve that is traced continuously **through** its own crossing point — a
figure-eight — stays one polyline here; it is broken into simple loops one level up (below).

**Self-crossing split.** [`geometry/IntersectionSplit.h`](../../OpenglGeometry/src/geometry/IntersectionSplit.h)
— `SplitSelfCrossings` scans each traced component for a pair of vertices far apart along the
curve both ways that nearly coincide in 3D (the curve revisiting a point). It cuts the loop
there into two simple closed loops. `GUI::CreateIntersection` runs this on the finder's
result and makes **one curve object per resulting loop**, so a figure-eight becomes two
ordinary closed intersection curves — each trims with the plain flood fill. Curves born from
a split carry `IntersectionCurveComponent::splitPiece`; a re-trace re-splits the fresh trace
and keeps whichever loop still matches (`MeshGeneratingSystem::IntersectionCurveGeneration`).

## The surface interface

```cpp
class IParametricSurface
{
    virtual Algebra::Vector4 Evaluate(float u, float v) const = 0;
    virtual Algebra::Vector4 DerivativeU(float u, float v) const = 0;
    virtual Algebra::Vector4 DerivativeV(float u, float v) const = 0;
    virtual bool WrappedU() const = 0;
    virtual bool WrappedV() const = 0;
    // provided: Normal(), Clamp() — clamp folds a wrapped axis and clamps a free one
};
```

`u`, `v` run over `[0, 1]`. CPU and GPU evaluation must agree or a traced curve floats off
the surface; both share their basis functions.

- **`TorusParametricSurface`** — closed form, wrapped both directions, matches
  `MeshGenerator::Torus::GetPoint` then the entity model matrix.
- **`BezierSurfaceParametricSurface`** — a grid of 4×4 Bernstein patches in world space;
  `u` across columns, `v` down rows, matching `bezierSurface/defaultHorizontal.tese`. C2
  surfaces pass through `DeBoorToBernstein` first. Wrapping is detected by sampling
  opposite edges.
- **`PointParametricSurface`** — a constant point.

## Scene integration

```
intersection curve entity
 ├─ ObjectTypeComponent { IntersectionCurve }
 ├─ IntersectionCurveComponent { surfaceP, surfaceQ, points, paramsP, paramsQ,
 │                               closed, wrapped flags, stepLength, precision,
 │                               useCursor, cursorPosition }
 ├─ ColorComponent, MeshComponent (GL_LINES)
 └─ (registered in every surface control point's NotificationComponent)
```

- [`ui/Intersections.h`](../../OpenglGeometry/src/ui/Intersections.h) — `GUI::CreateIntersection`,
  reached from **Create ▸ Intersection curve** (defaults) or **Intersection curve…**
  ([`popups/IntersectionDialog.cpp`](../../OpenglGeometry/src/ui/popups/IntersectionDialog.cpp) —
  step length / precision / *Use 3D cursor*), with one surface (self) or two selected. Calls
  `FindIntersections` and, if it returns any points, creates the curve object and registers it
  with each surface for notifications and trimming.
- [`MeshGeneratingSystem::IntersectionCurveGeneration`](../../OpenglGeometry/src/systems/MeshGeneratingSystem.cpp)
  — builds the polyline mesh; when `retraceRequested` is set (a control point moved, or the
  panel changed step length / precision) it calls `FindIntersections` again.
- [`ui/ParameterSpace.cpp`](../../OpenglGeometry/src/ui/ParameterSpace.cpp) — the **Parameter
  space** panel (View menu) draws the selected curve in the `(u, v)` square of each surface,
  breaking the polyline at seam crossings on wrapped axes. It carries the **step length** and
  **precision** ranges (both feed `IntersectionSettings`), a **Re-trace** button, and
  **Convert to interpolating C2**, which subsamples the curve to ~24 points and builds an
  `interpolatedC2` curve through them.

## Not serialised

Intersection curves have no representation in the scene format; `SaveScene` skips them with
a warning. The converted interpolating curve is an ordinary `interpolatedC2` object and
saves normally.

## Trimming

A surface can be limited to one side of a **closed** intersection curve.

1. [`TrimMask.h`](../../OpenglGeometry/src/geometry/TrimMask.h) rasterises the curve's
   parameter-space polyline into a `Globals::trimMaskResolution`² grid (512), seals the
   cells it passes through, then floods from a seed just inside the curve. Reached cells plus
   the curve are one region (255); everything else is 0. Wrapped axes let the curve and the
   fill wrap. The flood assumes a **simple** closed loop — a self-crossing curve is split
   before it gets here (see below), so each mask only ever sees a plain in/out boundary.
2. [`Texture2D`](../../OpenglGeometry/src/renderer/Texture2D.h) (RGBA8, nearest, clamp) holds
   the mask.
3. [`TrimmingComponent`](../../OpenglGeometry/src/scene/Components.h) on the surface lists the
   cutting curves, the active one, `enabled`, and `side`.
   `MeshGeneratingSystem::TrimMaskGeneration` rebuilds the mask when the curve changes, is
   deleted, or the side changes.
4. Both surface types feed a global `(u, v)` to a fragment shader that samples the mask and
   `discard`s the unwanted side:
   - **Bézier surfaces** — the vertex carries `(patchCol, patchRow, patchCols, patchRows)`;
     the tessellation shaders build the global `(u, v)`.
   - **Torus** — the mesh vertex carries `(u, v)` directly and draws with the dedicated
     `torusSurface` shader.
5. The **Trimming** row in the Parameter space panel: a *Trim &lt;surface&gt;* checkbox plus
   *keep filled* / *keep other*, per parent surface of the selected curve.

Not serialised — trimming state has no place in the scene format.
