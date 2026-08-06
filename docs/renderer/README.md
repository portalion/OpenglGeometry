# Renderer

[`src/renderer/`](../../OpenglGeometry/src/renderer/) is a thin, hand-rolled abstraction over
OpenGL 4.6. There is no render graph, no material system and no state cache — it is
deliberately small.

| File | Contents |
| --- | --- |
| `Renderer.{h,cpp}` | `RenderingMode` enum, the draw-call orchestrator |
| `RendererContext.h` | `SceneContext`, `EntityContext`, `UniformContext` |
| `Shader.{h,cpp}` | Program compilation, uniform reflection, uniform setters |
| `ShaderBuilder.{h,cpp}` | Fluent builder that loads GLSL files into a `Shader` |
| `VertexArray.{h,cpp}` | VAO + owned buffers |
| `VertexBuffer.{h,cpp}`, `IndexBuffer.{h,cpp}` | VBO / IBO wrappers |
| `Buffer.h` | `ShaderDataType`, `BufferElement`, `BufferLayout` |

Sub-pages: [buffers & vertex arrays](buffers.md) · [shader pipeline](shaders.md) ·
[shader reference](shader-reference.md)

---

## `RenderingMode`

```cpp
enum class RenderingMode
{
    Triangles = GL_TRIANGLES,
    Lines     = GL_LINES,
    Patches   = GL_PATCHES
};
```

Cast straight to `GLenum` at the draw call. `Patches` is mandatory for anything with
tessellation shaders (Bézier curves and surfaces).

## `Renderer`

```cpp
class Renderer
{
    Ref<Shader>      m_ActualShader;
    Ref<VertexArray> m_ActualMesh;
    UniformContext   sceneContext;
public:
    void SetShader(AvailableShaders shaderType);
    void SetSceneContext(SceneContext context);
    void SetMesh(Ref<VertexArray> mesh);
    void Render(RenderingMode mode, const EntityContext& context);
};
```

An immediate-mode, stateful API: set the scene context once per frame, then for each entity
set the mesh, set a shader, and render.

```cpp
void Renderer::SetShader(AvailableShaders shaderType)
{
    m_ActualShader = ShaderManager::GetInstance().GetShader(shaderType);
    m_ActualShader->Bind();     // also sets glPatchParameteri(GL_PATCH_VERTICES, patchSize)
}
```

Binding the shader is what applies its patch size, which is why `SetShader` must be called
before `Render` for tessellated geometry.

```cpp
void Renderer::SetSceneContext(SceneContext context)
{
    UniformContext tmp;
    tmp.Matrix4Uniforms["g_projectionMatrix"] = context.ProjectionMatrix;
    tmp.Matrix4Uniforms["g_viewMatrix"]       = context.ViewMatrix;
    tmp.Vector4Uniforms["g_cameraPosition"]   = context.CameraPosition;
    sceneContext = tmp;
}
```

```cpp
void Renderer::Render(RenderingMode mode, const EntityContext& context)
{
    if (!m_ActualShader) return;

    UniformContext uniformContext = sceneContext;                       // copy per draw
    uniformContext.Matrix4Uniforms["u_modelMatrix"] =
        context.Position * context.Rotation * context.Scale;
    uniformContext.Vector4Uniforms["u_color"] = Algebra::Vector4(1.f, 0.2f, 0.f, 1.f);

    m_ActualShader->ApplyContext(uniformContext);
    m_ActualMesh->Bind();
    glDrawElements(static_cast<GLenum>(mode),
                   m_ActualMesh->GetIndexBuffer()->GetCount(),
                   GL_UNSIGNED_INT, nullptr);
}
```

Notes:

- Model matrix order is **T · R · S** — scale applied first.
- `u_color` is a constant. There is no per-entity colour yet, although `Globals` already
  defines `defaultPointsColor` and `defaultMiddlePointColor` for a future one.
- `m_ActualMesh` is not null-checked; a `MeshComponent` with a null `mesh` crashes here.
- The `UniformContext` is copied and rebuilt on every single draw call. Not a problem at this
  scale, but it is the obvious first optimisation.

## The three contexts

[`RendererContext.h`](../../OpenglGeometry/src/renderer/RendererContext.h)

```cpp
struct SceneContext          // once per frame, from the active camera
{
    Algebra::Matrix4 ViewMatrix;
    Algebra::Matrix4 ProjectionMatrix;
    Algebra::Vector4 CameraPosition;
};

struct EntityContext         // once per entity, filled by UniformApplier
{
    Algebra::Matrix4 Position = Algebra::Matrix4::Identity();
    Algebra::Matrix4 Rotation = Algebra::Matrix4::Identity();
    Algebra::Matrix4 Scale    = Algebra::Matrix4::Identity();
};

struct UniformContext        // name → value, what actually reaches the GPU
{
    std::unordered_map<std::string, Algebra::Vector4> Vector4Uniforms;
    std::unordered_map<std::string, Algebra::Matrix4> Matrix4Uniforms;
};
```

The split is: `SceneContext` and `EntityContext` are *typed and semantic*; `UniformContext`
is *stringly-typed and generic*, matched against the shader's reflected uniform list.

**`UniformContext` only supports `vec4` and `mat4`.** Shaders that declare `int`, `float` or
`vec2` uniforms cannot be fed from the context — they keep their GLSL default value and
`Shader::ApplyContext` prints a warning every frame. The Bézier surface shaders'
`u_subdivisions`/`v_subdivisions` are exactly this case. Extending `UniformContext` with an
`IntUniforms` map (plus a `case GL_INT:` in `ApplyContext`) is the fix; see
[shaders.md](shaders.md#adding-a-new-uniform-type).

## Frame sequence

```
App::Run
 └─ glClear(COLOR | DEPTH)
    ImGui new frame
    SystemPipeline::Update
     └─ RenderingSystem::Process
         ├─ for each active CameraComponent:
         │     ICamera::HandleInput  → viewMatrix
         │     UniformApplier        → camera EntityContext
         │     Renderer::SetSceneContext
         └─ for each MeshComponent (not IsInvisibleTag):
               UniformApplier        → entity EntityContext
               Renderer::SetMesh
               for each shader in MeshComponent::shaderTypes:
                   Renderer::SetShader   (binds program, sets patch size)
                   Renderer::Render      (uniforms + glDrawElements)
    ImGui render + draw
    glfwSwapBuffers / glfwPollEvents
```

## GL state

Global state is set once in
[`Window::SetupGLFWFunctions`](../../OpenglGeometry/src/core/Window.cpp):

```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

Blending is on for the infinite grid's distance fade. **Depth testing is never enabled**,
even though the depth buffer is cleared each frame — fine for the current all-wireframe look,
but something to turn on before rendering solid geometry.

The viewport is set in `Window::HandleResize` to
`max(width - Globals::rightInterfaceWidth, 0) × height`, reserving 400 px on the right for
the ImGui panels.

## Error checking

[`utils/GlCall.h`](../../OpenglGeometry/src/utils/GlCall.h):

```cpp
#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__));
```

Wrap GL calls in `GLCall(...)` and a failing call breaks into the debugger with the function,
file and line printed. It is used consistently in `Shader`, `Window` and the buffer classes,
though not in `Renderer::Render` itself.

Note the macro is **not** compiled out in release builds and `__debugbreak()` is MSVC-specific.
It also expands to multiple statements, so never use it as the body of an unbraced `if`.
