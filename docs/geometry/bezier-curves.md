# Bézier curves

Three curve types share one component, one system pass and one shader program. They differ
only in the `std::function` stored in `BezierLineGenerationComponent::generationFunction`.

| Curve | Generator | Passes through its control points? | Control polyline? |
| --- | --- | --- | --- |
| **Bézier C0** | `MeshGenerator::BezierCurveC0::GenerateVertices` | Only the segment joints | yes (virtual) |
| **Bézier C2** | `MeshGenerator::BezierCurveC2::GenerateVertices` | no (B-spline hull) | yes (virtual) |
| **Interpolated** | `MeshGenerator::InterpolatedBezierCurve::GenerateVertices` | yes, all of them | no |

All three return the same thing: a flat `std::vector<Algebra::Vector4>` of Bézier control
points in groups of four, which the tessellation shader evaluates as consecutive cubic
segments. See [renderer/shader-reference.md](../renderer/shader-reference.md#beziercurvec0).

---

## Bézier C0

[`BezierCurveGenerator.cpp`](../../OpenglGeometry/src/meshGenerators/BezierCurveGenerator.cpp)

A chain of cubic Bézier segments joined end to end. Continuity is C0 — position is continuous
at the joints, the tangent generally is not.

The control points are consumed 4 at a time with the last point of each segment shared as
the first point of the next: `P0 P1 P2 P3 | P3 P4 P5 P6 | …`. `GenerateVertices` duplicates
every 3rd point to produce that:

```cpp
result.push_back(controlPoints[0]);

for (size_t i = 1; i < controlPoints.size(); i++)
{
    result.push_back(controlPoints[i]);
    if (i % 3 == 0)
        result.push_back(controlPoints[i]);   // shared joint, emitted twice
}
```

### `FixVertices` — padding a partial last segment

The GPU needs a multiple of 4. `FixVertices` pads whatever is left over, choosing a rule per
remainder so the tail looks reasonable rather than degenerate:

| `size % 4` | Handling |
| --- | --- |
| 0 | nothing to do |
| 1 | repeat the last point three times → a degenerate segment (renders as a dot) |
| 2 | `p0 p1` → `p0 p0 p1 p1` → a straight line |
| 3 | `p0 p1 p2` → `p0, ⅓p0+⅔p1, ⅔p1+⅓p2, p2` → the exact cubic elevation of a quadratic Bézier |

The remainder-3 case is degree elevation: a quadratic Bézier with control points
`p0, p1, p2` is exactly the cubic with the four points above.

An empty control-point list returns an empty vector before any of this.

---

## Bézier C2 (cubic B-spline)

Also in `BezierCurveGenerator.cpp`. The user's points are **B-spline (de Boor) control
points**; the curve is C2-continuous and generally does *not* pass through them.

The generator converts each window of four de Boor points into the equivalent Bézier
segment:

```cpp
for (size_t i = 0; i + 3 < bSplineControlPoints.size(); i++)
{
    const auto& P0 = bSplineControlPoints[i];
    const auto& P1 = bSplineControlPoints[i + 1];
    const auto& P2 = bSplineControlPoints[i + 2];
    const auto& P3 = bSplineControlPoints[i + 3];

    Algebra::Vector4 B0 = (P0 + 4.0f * P1 + P2) / 6.0f;
    Algebra::Vector4 B1 = (      4.0f * P1 + 2.0f * P2) / 6.0f;
    Algebra::Vector4 B2 = (      2.0f * P1 + 4.0f * P2) / 6.0f;
    Algebra::Vector4 B3 = (P1 + 4.0f * P2 + P3) / 6.0f;

    result.insert(result.end(), { B0, B1, B2, B3 });
}
```

This is the standard uniform cubic B-spline → Bézier basis conversion. The interior control
points `B1`, `B2` sit at ⅓ and ⅔ along the `P1P2` edge; the endpoints `B0`, `B3` are the
B-spline evaluations at the knots.

Fewer than 4 points returns empty — a C2 B-spline needs at least one full window. The result
is always a multiple of 4, so no padding is needed.

Note that consecutive windows share `P1`/`P2`, so `B3` of one segment equals `B0` of the
next — that is exactly what makes the joins C2.

---

## Interpolated Bézier (natural cubic spline)

[`InterpolatedBezierCurveGenerator.cpp`](../../OpenglGeometry/src/meshGenerators/InterpolatedBezierCurveGenerator.cpp)

The only curve that actually **passes through every control point**. It builds a natural
cubic spline with chord-length parameterisation, then converts each polynomial piece to
Bézier form.

### Degenerate cases

```cpp
if (controlPoints.size() < 2) return { };
if (controlPoints.size() == 2)
    return { controlPoints[0], controlPoints[0], controlPoints[1], controlPoints[1] };
```

Two points give a straight line expressed as a cubic.

### The algorithm

1. **Chord lengths** — `d[i] = |P[i+1] - P[i]|`, the parameter interval of each piece.

2. **Tridiagonal system** — build the sub-diagonal `alpha`, super-diagonal `beta` and
   right-hand side `r`:

   ```cpp
   alpha.push_back(d0 / (d0 + d1));                    // skipped for i == 1
   beta .push_back(d1 / (d0 + d1));                    // skipped for the last i
   r    .push_back(3.f * (P1 - P0) / (d0 + d1));       // P0, P1 are divided differences
   ```

   NaNs from zero-length chords (two coincident points) are scrubbed to 0 before solving.

3. **Solve** — `Algebra::Matrix4::SolveTrilinealEquation(alpha, beta, r)` returns the
   interior second-derivative coefficients `c`. Zeros are prepended and appended, which is
   the **natural** boundary condition (zero curvature at both ends).

4. **Power-basis coefficients** — for each piece, `a` is the interpolated point, and

   ```cpp
   D[i] = (c[i + 1] - c[i]) / d[i] / 3.f;                            // cubic term
   b[i] = (a[i + 1] - a[i]) / d[i] - c[i] * d[i] - D[i] * d[i] * d[i];
   ```

5. **Power basis → Bézier** — the standard conversion, with the parameter rescaled to
   `[0, 1]` per piece:

   ```cpp
   Algebra::Vector4 ai = a[i];
   Algebra::Vector4 bi = b[i] * d[i];
   Algebra::Vector4 ci = c[i] * d[i] * d[i];
   Algebra::Vector4 di = D[i] * d[i] * d[i] * d[i];

   P0 = ai;
   P1 = ai + bi / 3.f;
   P2 = ai + bi * 2.f / 3.f + ci / 3.f;
   P3 = ai + bi + ci + di;
   P0.w = P1.w = P2.w = P3.w = 1.0f;
   ```

   The explicit `w = 1` matters: the intermediate arithmetic operates on all four components,
   so `w` drifts and must be restored before the points are used as homogeneous positions.

The solver itself lives in the Algebra library — see [algebra.md](../algebra.md).

---

## How the archetypes differ

[`BezierCurveArchetypeCreation.h`](../../OpenglGeometry/src/archetypes/BezierCurveArchetypeCreation.h)

C0 and C2 both create a **virtual control polyline** so you can see the control hull:

```cpp
entity.AddTag<IsDirtyTag>();

auto virtualPolyline = scene->CreateEntity();
AddVirtualToEntity(virtualPolyline, entity);           // helper of the curve
AddPolylineToEntity(virtualPolyline, pointsBegin, pointsEnd);

AddLineToEntity(entity, pointsBegin, pointsEnd);       // the curve's own control points
auto& bezierComponent = entity.AddComponent<BezierLineGenerationComponent>();
bezierComponent.generationFunction = MeshGenerator::BezierCurveC0::GenerateVertices;
```

The polyline can be hidden with the checkbox `ShapeInspectorSystem::VirtualInspect` renders.

[`InterpolatedBezierCurveArchetypeCreation.h`](../../OpenglGeometry/src/archetypes/InterpolatedBezierCurveArchetypeCreation.h)
omits the virtual polyline — the curve already visits every point, so a hull adds nothing.

---

## Adding a fourth curve type

The cheapest extension in the codebase — no new component, no system change:

1. Write the generator:

```cpp
// meshGenerators/MyCurveGenerator.h
namespace MeshGenerator { namespace MyCurve {
    std::vector<Algebra::Vector4> GenerateVertices(const std::vector<Algebra::Vector4>&);
}}
```

   Return a multiple of 4 control points (pad like `FixVertices` if necessary).

2. Include it in `MeshGenerators.h`. The build picks the files up automatically.

3. Copy `BezierCurveArchetypeCreation.h`'s pattern into a new archetype, setting
   `generationFunction = MeshGenerator::MyCurve::GenerateVertices` and a distinct shape name.

4. Add a `ImGui::MenuItem` to `ShapeCreation::Display`.
