# How to add a new shader

## 1. Write the GLSL

Files go under `OpenglGeometry/resources/shaders/`, optionally in a subdirectory when a
program has several stages. Extensions are fixed by
[`Shader::shaderInfoMap`](../renderer/shaders.md#shadertype):

| Stage | Extension |
| --- | --- |
| Vertex | `.vert` |
| Fragment | `.frag` |
| Geometry | `.geom` |
| Tessellation control | `.tesc` |
| Tessellation evaluation | `.tese` |

A minimal pair, `resources/shaders/myShader.vert`:

```glsl
#version 460 core

layout(location = 0) in vec4 position;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;

void main()
{
    gl_Position = g_projectionMatrix * g_viewMatrix * u_modelMatrix * position;
}
```

`resources/shaders/myShader.frag`:

```glsl
#version 460 core

layout(location = 0) out vec4 color;

uniform vec4 u_color = vec4(1.f, 0.2f, 0.f, 1.f);

void main()
{
    color = u_color;
}
```

Rules to respect:

- **Attribute locations come from `BufferLayout` element order**, not from names. A mesh
  built with `{ Float4 "position", Float3 "color" }` must declare
  `layout(location = 0) in vec4 …; layout(location = 1) in vec3 …;`.
- **Only `vec4` and `mat4` uniforms can be set from C++.** Any other type keeps its GLSL
  default silently. Give defaults to everything so a missing value is not a black screen.
- **Prefix uniforms**: `g_` for scene-wide, `u_` for per-object.
- The available uniforms today are `g_viewMatrix`, `g_projectionMatrix`,
  `g_cameraPosition`, `u_modelMatrix`, `u_color`. Anything else needs a
  [context change](../renderer/shaders.md#adding-a-new-uniform).

## 2. Add the enum entry

`src/managers/ShaderManager.h`:

```cpp
enum class AvailableShaders
{
    Default                 = 0,
    InfiniteGrid            = 1,
    BezierCurveC0           = 2,
    BezierSurfaceHorizontal = 3,
    BezierSurfaceVertical   = 4,
    Point                   = 5,
    Cursor                  = 6,
    MyShader                = 7      // ← new
};
```

## 3. Register the program

`src/managers/ShaderManager.cpp`, in the constructor:

```cpp
AssignShader(AvailableShaders::MyShader, ShaderBuilder("resources/shaders/")
    .AddShader(ShaderType::Vertex,   "myShader")
    .AddShader(ShaderType::Fragment, "myShader"));
```

The builder takes a **directory** (trailing slash) and each `AddShader` takes a **stem**;
the extension is appended automatically.

For a tessellated program, add the stages and set the patch size:

```cpp
AssignShader(AvailableShaders::MyTessShader, ShaderBuilder("resources/shaders/myTess/")
    .AddShader(ShaderType::Vertex,                "default")
    .AddShader(ShaderType::TesselationControl,    "default")
    .AddShader(ShaderType::TesselationEvaluation, "default")
    .AddShader(ShaderType::Fragment,              "default")
    .ChangePatchSize(4));
```

`ChangePatchSize(n)` must match the number of control points per primitive: 4 for cubic
curves, 16 for bicubic patches.

## 4. Use it on a mesh

Either directly on a `MeshComponent`:

```cpp
auto& meshComponent = entity.AddComponent<MeshComponent>();
meshComponent.mesh = /* ... */;
meshComponent.shaderTypes = { AvailableShaders::MyShader };
meshComponent.renderingMode = RenderingMode::Lines;
```

or from a generation pass:

```cpp
ModifyOrCreateMesh(entity, vertices, indices, layout,
                   RenderingMode::Patches, { AvailableShaders::MyShader });
```

`shaderTypes` is a list, so a mesh can be drawn several times with different programs — that
is how Bézier surfaces get isolines in both directions:

```cpp
{ AvailableShaders::BezierSurfaceHorizontal, AvailableShaders::BezierSurfaceVertical }
```

**Tessellated shaders require `RenderingMode::Patches`.**

## 5. Rebuild

Shaders are copied next to the executable by the post-build step:

```cmake
add_custom_command(TARGET OpenglGeometry POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/resources"
            "$<TARGET_FILE_DIR:OpenglGeometry>/resources")
```

Editing a shader and pressing F5 without a rebuild runs the **old** copy. There is no hot
reload. While iterating on a shader it is often quicker to edit the copy under
`out/build/<name>/OpenglGeometry/resources/` and just restart — remember to copy your
changes back to the source tree.

---

## Debugging

| Symptom | Likely cause |
| --- | --- |
| Nothing renders, no errors | Shader file path wrong. `ShaderBuilder::ParseShaderCode` does not check that the file opened; an empty source compiles to an empty shader and links "successfully" |
| `Failed to compile fragment shader!` for a `.tesc` file | The message only distinguishes vertex from "fragment" — read the info log body, not the header |
| `WARNING: There is no uniform named: X` every frame | The shader declares `X` but nothing puts it into the `UniformContext`. Add it in `Renderer::SetSceneContext` or `Renderer::Render` |
| `Warning: uniform 'X' doesn't exist!` | `glGetUniformLocation` returned -1: either misspelled, or declared-but-unused so the compiler stripped it |
| A uniform is silently never set | Its type is not `vec4` or `mat4`. `Shader::ApplyContext` handles only those two and falls through for everything else — no warning |
| Tessellated geometry draws as points/lines | `RenderingMode` is not `Patches`, or `ChangePatchSize` does not match the vertices per primitive |
| Attributes read garbage | `BufferLayout` element order does not match the `layout(location = …)` declarations |

Console output at startup lists shader loading (`INFO: Loading Shaders` /
`INFO: Loaded Shaders`) and the GL/GLSL versions.

Full reference for the existing programs:
[renderer/shader-reference.md](../renderer/shader-reference.md).
