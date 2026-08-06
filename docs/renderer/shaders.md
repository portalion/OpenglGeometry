# The shader pipeline

Three classes cooperate: [`ShaderBuilder`](#shaderbuilder) loads GLSL files,
[`Shader`](#shader) compiles/links/reflects them, and
[`ShaderManager`](../managers.md#shadermanager) owns one instance of each program.

For what each shader program actually computes, see [shader-reference.md](shader-reference.md).

---

## `ShaderType`

```cpp
enum class ShaderType
{
    Vertex = 1, Fragment = 2, Geometry = 3,
    TesselationControl = 4, TesselationEvaluation = 5
};
```

Mapped to file extensions and GL constants by a static table in `Shader.cpp`:

```cpp
const std::unordered_map<ShaderType, ShaderTypeInfo> Shader::shaderInfoMap =
{
    { ShaderType::Vertex,                { ".vert", GL_VERTEX_SHADER          }},
    { ShaderType::Fragment,              { ".frag", GL_FRAGMENT_SHADER        }},
    { ShaderType::Geometry,              { ".geom", GL_GEOMETRY_SHADER        }},
    { ShaderType::TesselationControl,    { ".tesc", GL_TESS_CONTROL_SHADER    }},
    { ShaderType::TesselationEvaluation, { ".tese", GL_TESS_EVALUATION_SHADER }},
};
```

So a shader stage's file extension is derived, never spelled out at the call site.

---

## `ShaderBuilder`

[`ShaderBuilder.h`](../../OpenglGeometry/src/renderer/ShaderBuilder.h) ·
[`.cpp`](../../OpenglGeometry/src/renderer/ShaderBuilder.cpp)

```cpp
ShaderBuilder("resources/shaders/bezierLine/")
    .AddShader(ShaderType::Vertex,                "default")
    .AddShader(ShaderType::TesselationControl,    "default")
    .AddShader(ShaderType::TesselationEvaluation, "default")
    .AddShader(ShaderType::Fragment,              "default")
    .ChangePatchSize(4);
```

- The constructor takes a **directory** (with trailing slash); `AddShader` takes a
  **stem**. The full path is `directory + filename + extension`.
- Paths are relative to the working directory, so the executable must run from the directory
  containing `resources/` — see [getting-started](../getting-started.md).
- `ChangePatchSize(n)` sets `glPatchParameteri(GL_PATCH_VERTICES, n)` at bind time.
  Default 4. Curves use 4, surfaces use 16.
- `Build()` returns a `Shader` by value; `BuildShared()` returns `std::shared_ptr<Shader>` —
  `ShaderManager` uses the latter.
- Adding the same stage twice logs a warning and overwrites.

> `ParseShaderCode` does not check whether the file opened. A missing or misspelled path
> yields an empty source string, which compiles to an empty shader; the program links but
> draws nothing. See [gotchas](../gotchas.md).

---

## `Shader`

[`Shader.h`](../../OpenglGeometry/src/renderer/Shader.h) ·
[`.cpp`](../../OpenglGeometry/src/renderer/Shader.cpp)

```cpp
Shader(const std::unordered_map<ShaderType, std::string>& sourceCodes,
       unsigned int patchSize = 4);
~Shader();                                  // glDeleteProgram

void ApplyContext(UniformContext context);
void Bind() const;
void UnBind() const;
```

### Construction

```cpp
m_RendererID = CreateShader(sourceCodes);   // compile each stage, attach, link, validate
ReflectUniforms(m_RendererID);              // enumerate active uniforms
```

`CompileShader` prints the info log on failure and returns 0. Note the failure message only
distinguishes vertex from "fragment", so a broken tessellation shader is reported as a
fragment shader error — check the log body, not the header.

`CreateShader` does not check the link or validate status, so a link failure is silent.

### `Bind`

```cpp
void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
    GLCall(glPatchParameteri(GL_PATCH_VERTICES, patchSize));
}
```

Patch size is applied on every bind, which is why `Renderer::SetShader` must be called before
`Render` for tessellated meshes — and why a mesh listing two shaders with different patch
sizes is fine.

### Uniform reflection

```cpp
void Shader::ReflectUniforms(unsigned int program)
{
    GLint count;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);

    for (GLint i = 0; i < count; i++)
    {
        char name[256]; GLsizei length; GLint size; GLenum type;
        glGetActiveUniform(program, i, sizeof(name), &length, &size, &type, name);
        m_Uniforms.push_back({ std::string(name, length), type, GetUniformLocation(name) });
    }
}
```

The program is asked what uniforms it actually has, once, at construction. This is the
inverse of the usual "set uniform by name and hope" approach: the shader drives, and the
`UniformContext` is queried for each declared uniform.

Important consequence: **the GLSL compiler strips unused uniforms**. A uniform declared but
not contributing to the output does not appear in `m_Uniforms` and is never set.

### `ApplyContext`

```cpp
void Shader::ApplyContext(UniformContext context)
{
    for (const auto& uniform : m_Uniforms)
    {
        switch (uniform.type)
        {
        case GL_FLOAT_VEC4:
            if (context.Vector4Uniforms.contains(uniform.name))
                SetUniformVec4f(uniform.name, context.Vector4Uniforms[uniform.name]);
            else
                std::cout << "WARNING: There is no uniform named: " << uniform.name << ...;
            break;
        case GL_FLOAT_MAT4:
            if (context.Matrix4Uniforms.contains(uniform.name))
                SetUniformMat4f(uniform.name, context.Matrix4Uniforms[uniform.name]);
            else
                std::cout << "WARNING: ...";
            break;
        }
    }
}
```

Only `vec4` and `mat4` are handled. **Any other declared uniform type falls through the
switch silently** — no warning, no value set. Uniform types that *are* handled but missing
from the context print a warning every frame, which is how you find out about them.

`SetUniformMat4f` passes `GL_TRUE` for `transpose`, because `Algebra::Matrix4` is
row-major while GLSL expects column-major.

### Uniform naming convention

| Prefix | Meaning | Filled by |
| --- | --- | --- |
| `g_` | scene-global, constant for the frame | `Renderer::SetSceneContext` |
| `u_` | per-object | `Renderer::Render` |

Current set:

| Uniform | Type | Value |
| --- | --- | --- |
| `g_projectionMatrix` | `mat4` | active camera's `projectionMatrix` |
| `g_viewMatrix` | `mat4` | active camera's `viewMatrix` |
| `g_cameraPosition` | `vec4` | camera translation row (see [gotchas](../gotchas.md)) |
| `u_modelMatrix` | `mat4` | `Position * Rotation * Scale` |
| `u_color` | `vec4` | hard-coded `(1, 0.2, 0, 1)` |

### `GetUniformLocation`

Caches name → location. Locations of `-1` (uniform not found) print a warning and are **not**
cached, so a genuinely missing uniform re-queries GL and re-warns on every call. Note the
comparison is done on an `unsigned int`, so `-1` matches only by wraparound.

---

## Adding a new uniform

### …of an existing type (`vec4` / `mat4`)

1. Declare it in the GLSL with the right prefix, and *use it* (unused uniforms are optimised
   away).
2. Put it into the context. Scene-wide → `Renderer::SetSceneContext`. Per-object →
   `Renderer::Render`, or derive it from a component via
   [`UniformApplier`](../ecs/component-function-registry.md):

```cpp
// UniformApplier.h — extend EntityContext first if you need a new field
void ColorApplier(Entity entity, EntityContext& context);

// UniformApplier.cpp
UniformApplier::UniformApplier()
{
    ...
    Bind<ColorComponent>(&UniformApplier::ColorApplier);
}
```

If the value is per-entity, remember to also carry it from `EntityContext` into the
`UniformContext` inside `Renderer::Render`.

### …of a new type

`ApplyContext` and `UniformContext` must both learn about it. For `int`:

```cpp
// RendererContext.h
struct UniformContext
{
    std::unordered_map<std::string, Algebra::Vector4> Vector4Uniforms;
    std::unordered_map<std::string, Algebra::Matrix4> Matrix4Uniforms;
    std::unordered_map<std::string, int>              IntUniforms;   // new
};

// Shader.cpp — ApplyContext
case GL_INT:
    if (context.IntUniforms.contains(uniform.name))
        SetUniformVec1i(uniform.name, context.IntUniforms[uniform.name]);
    break;
```

`SetUniformVec1i` already exists and is currently unused. Doing this would let the Bézier
surface subdivision counts become editable instead of frozen at their GLSL defaults.

---

## Adding a whole new shader program

See [how-to/add-a-new-shader.md](../how-to/add-a-new-shader.md). In short: write the GLSL
under `resources/shaders/`, add an `AvailableShaders` enum entry, register it in
`ShaderManager`'s constructor, and reference it from a `MeshComponent::shaderTypes`.
