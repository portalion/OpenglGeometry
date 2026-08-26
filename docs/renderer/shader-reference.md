# Shader reference

Every GLSL program in [`OpenglGeometry/resources/shaders/`](../../OpenglGeometry/resources/shaders/),
registered in [`ShaderManager`](../managers.md#shadermanager).

| `AvailableShaders` | Directory | Stages | Patch size |
| --- | --- | --- | --- |
| `Default` | `shaders/` | vert, frag | 4 (unused) |
| `InfiniteGrid` | `shaders/` | vert, frag | 4 (unused) |
| `Point` | `shaders/` | vert (`point`), frag (`default`) | 4 (unused) |
| `Cursor` | `shaders/` | vert, frag (`cursor`) | 4 (unused) |
| `BezierCurveC0` | `shaders/bezierLine/` | vert, tesc, tese, frag | 4 |
| `BezierSurfaceHorizontal` | `shaders/bezierSurface/` | vert, tesc, tese, frag | 16 |
| `BezierSurfaceVertical` | `shaders/bezierSurface/` | vert, tesc, tese, frag | 16 |

---

## `Default`

`default.vert` + `default.frag`. The standard transform-and-flat-colour pair, used by the
torus, curves' control polylines and anything created through `ModifyOrCreateMesh` without an
explicit shader. Points use their own vertex stage (`Point` below) but share this
`default.frag`; the cursor has its own vert **and** frag.

```glsl
// default.vert
layout(location = 0) in vec4 position;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;

void main()
{
    gl_Position = g_projectionMatrix * g_viewMatrix * u_modelMatrix * position;
}
```

```glsl
// default.frag
uniform vec4 u_color = vec4(1.f, 0.2f, 0.f, 1.f);
void main() { color = u_color; }
```

Expects the `{ Float4 "position" }` layout. Works with any `RenderingMode`.

---

## `Point`

`point.vert` + `default.frag`, used by every point (`Archetypes::AddPointToEntity`), including
Bézier-surface control points.

```glsl
// point.vert
const float c_screenScale = 0.2;   // on-screen size, ~fraction of viewport half-height

void main()
{
    vec4 originView = g_viewMatrix * u_modelMatrix * vec4(0.0, 0.0, 0.0, 1.0);
    float distance  = max(-originView.z, 0.0001);
    originView.xy  += position.xy * distance * c_screenScale;   // view-plane offset
    gl_Position = g_projectionMatrix * originView;
}
```

The entity origin goes into view space, then the square mesh's local `xy` offset is applied
**in view space** rather than model space — so the quad is always parallel to the screen (a
camera-facing billboard) — and stretched by the origin's depth so the perspective `1/z` shrink
cancels out. Points face the camera **and** hold a constant screen size at any zoom. Adjust
`c_screenScale` in the shader to resize them.

---

## `Cursor`

`cursor.vert` + `cursor.frag`, used only by the cursor entity
(`Archetypes::CreateCursor`, `StaticMeshType::Cursor`, `RenderingMode::Lines`).

The cursor mesh carries a per-vertex colour — one line per axis, **X red, Y green, Z blue** —
so its layout is `{ Float4 "position", Float3 "color" }` (6 vertices, not the old shared-origin
4). `cursor.vert` passes the colour through; `cursor.frag` outputs it, multiplied by `u_color`
(the white `ColorComponent`) as a global tint.

```glsl
// cursor.vert
layout(location = 1) in vec3 color;
out vec3 v_color;

const float c_screenScale = 0.15;   // on-screen size, ~fraction of viewport half-height

void main()
{
    vec4 originView = g_viewMatrix * u_modelMatrix * vec4(0.0, 0.0, 0.0, 1.0);
    vec3 axisView   = mat3(g_viewMatrix * u_modelMatrix) * position.xyz;
    float distance  = max(-originView.z, 0.0001);

    vec4 viewPos = originView + vec4(axisView * distance * c_screenScale, 0.0);
    gl_Position  = g_projectionMatrix * viewPos;
    v_color = color;
}
```

Each axis-line offset keeps its (view-rotated) world orientation but is stretched by the
origin's view-space depth, which cancels the perspective `1/z` shrink — the cursor projects to
a **constant screen size** no matter how far the camera orbits or zooms. Adjust `c_screenScale`
in the shader to make it bigger or smaller.

---

## `InfiniteGrid`

`infiniteGrid.vert` + `infiniteGrid.frag`, used by the single grid entity created in
`BaseScene` with `StaticMeshType::Grid` and `RenderingMode::Lines`.

The mesh is a finite 200×200 line grid in the range [-1, 1] with per-vertex colour (grey,
lighter every 10th line, red on the centre line). The shader turns it into an apparently
infinite grid that rescales with camera distance.

### Vertex stage

```glsl
float dist      = max(abs(g_cameraPosition.z), 1e-6);
float log10dist = log(dist) / log(10.0);
float decade    = floor(log10dist);
float stepMul   = pow(10.0, decade);

scalling = stepMul * 10;          // grid spacing snaps to the current power of ten
fraction = log10dist - decade;    // 0..1 position within the decade → fade factor

out_worldPos.xyz *= scalling;

vec2 movement = floor(g_cameraPosition.xy / 10);
out_worldPos.xy -= movement * 10;  // slide the patch so it follows the camera

gl_Position = g_projectionMatrix * g_viewMatrix * out_worldPos;
```

Two tricks: the grid **scales** by the power of ten nearest the camera height, and it
**translates** in steps of 10 units so the finite mesh always surrounds the viewer. `fraction`
carries how far through the decade the camera is, used to cross-fade as the scale changes.

### Fragment stage

```glsl
float fadeDistance = abs(g_cameraPosition.z) * heightToFadeDistanceRatio;   // 15.0
fadeDistance = min(fadeDistance, in_scalling * 0.05f);
fadeDistance = max(fadeDistance, in_scalling * 0.5f);

float distanceToCamera = length(in_worldPos.xy - g_cameraPosition.xy);
float opacityFalloff   = smoothstep(1.0, 0.0, distanceToCamera / fadeDistance);

out_color = vec4(in_color, opacityFalloff * (1.0 - in_fraction));
```

Fades to transparent at the horizon. Requires blending, which
`Window::SetupGLFWFunctions` enables.

Both stages depend heavily on `g_cameraPosition`, which is currently always `(0,0,0,1)` —
see [gotchas](../gotchas.md). Fixing that changes how the grid looks.

Layout: `{ Float4 "position", Float3 "color" }`. Declares a local `maxFadeDistance` constant
that is never used.

---

## `BezierCurveC0`

`bezierLine/{default.vert, default.tesc, default.tese, default.frag}`, patch size 4.
Used for **all three curve types** — C0, C2 and interpolated — because the CPU-side generator
has already converted the control points into cubic Bézier segments.

### Vertex

Pass-through: `gl_Position = position;`. Control points stay in world space so the
tessellation control shader can measure them.

### Tessellation control

```glsl
layout(vertices = 4) out;
out patch float maxU;
```

Chooses the tessellation level adaptively from two signals:

```glsl
float screenDistance(vec4 a, vec4 b)     // clip → NDC → 2D length
float averageDistanceToCamera()          // mean world distance of the 4 control points

float distanceFactor = clamp(100.0 / distToCamera, 0.5, 2.0);
float scale = 32.0 * distanceFactor * 4;

gl_TessLevelOuter[0] = ceil(sqrt(d1 * scale));
gl_TessLevelOuter[1] = ceil(sqrt(d1 * scale));
maxU = gl_TessLevelOuter[0];
```

A curve that covers more screen space, or is closer to the camera, gets more segments.
`d0`, `d2` and `d3` are computed but only `d1` (the middle segment) feeds the level.

### Tessellation evaluation

```glsl
layout(isolines, equal_spacing, ccw) in;

vec4 Bezier(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float t)  // Bernstein basis
{
    float u = 1.0 - t;
    return u*u*u*p0 + 3.0*t*u*u*p1 + 3.0*t*t*u*p2 + t*t*t*p3;
}

gl_Position = g_projectionMatrix * g_viewMatrix * Bezier(p0, p1, p2, p3, u / maxU + v);
```

The `u / maxU + v` parameter is the standard isoline trick: with `isolines`, `gl_TessCoord.x`
runs `0 .. (n-1)/n` and `gl_TessCoord.y` selects the line, so dividing by `maxU` and adding
`v` reconstructs a continuous `t ∈ [0, 1]` and closes the gap at the segment end.

### Fragment

Flat `u_color` (default blue, overwritten to orange by `Renderer::Render`).

---

## `BezierSurfaceHorizontal` / `BezierSurfaceVertical`

`bezierSurface/{default.vert, defaultHorizontal.*, defaultVertical.*, default.frag}`,
patch size 16. Both are attached to the same mesh so the surface is drawn twice, giving
isolines in both parametric directions — the wireframe grid look.

The vertex and fragment stages are identical to the curve ones (pass-through + flat colour).

### Control stage

```glsl
layout (vertices=16) out;

uniform int u_subdivisions = 32;
uniform int v_subdivisions = 32;

// horizontal
gl_TessLevelOuter[0] = ceil(u_subdivisions + 1);
gl_TessLevelOuter[1] = ceil(v_subdivisions);

// vertical (guarded by `if (gl_InvocationID == 0)`)
gl_TessLevelOuter[0] = ceil(v_subdivisions + 1);
gl_TessLevelOuter[1] = ceil(u_subdivisions);
```

Fixed subdivision, not adaptive — unlike the curve shader.

**Both subdivision uniforms are `int`, and `UniformContext` has no int support**, so
`Shader::ApplyContext` never sets them and they keep their GLSL default of 32. They are
effectively compile-time constants today. Making them adjustable requires the change
described in [shaders.md](shaders.md#adding-a-new-uniform-type).

### Evaluation stage

Both evaluate a bicubic Bézier patch as four curves in one direction followed by one curve
through the results:

```glsl
vec3 row0 = interpolateRow(gl_in[0..3],  u);
vec3 row1 = interpolateRow(gl_in[4..7],  u);
vec3 row2 = interpolateRow(gl_in[8..11], u);
vec3 row3 = interpolateRow(gl_in[12..15],u);
vec3 finalPos = interpolateRow(row0, row1, row2, row3, v);
```

The only difference is which of `gl_TessCoord.x/.y` plays the role of `u` and which of `v`,
plus the same `(sub + 1) * t / sub` gap-closing rescale used by the curve shader.

Control points are consumed in **row-major order**, matching how
`MeshGeneratingSystem::BezierSurfaceGeneration` pushes `controlPoints[i][j]` with `j`
innermost.

### Version mismatch

`defaultHorizontal.*` are `#version 460 core`, `defaultVertical.*` are `#version 430`
(no `core` profile keyword). Both link on a 4.6 context, but it is an inconsistency worth
normalising if you touch these files.

---

## Writing a new shader

- Every shader that transforms geometry needs `g_viewMatrix` and `g_projectionMatrix`;
  object-space geometry also needs `u_modelMatrix`.
- Only `vec4` and `mat4` uniforms can be driven from the C++ side today.
- Attribute locations come from `BufferLayout` element order, not from the names.
- Tessellated shaders need `RenderingMode::Patches` on the `MeshComponent` and a matching
  `ChangePatchSize(n)` in the `ShaderManager` registration.
- Rebuild after editing GLSL so the post-build copy step refreshes the files next to the exe.

Step-by-step: [how-to/add-a-new-shader.md](../how-to/add-a-new-shader.md).
