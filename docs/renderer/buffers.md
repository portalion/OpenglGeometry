# Buffers and vertex arrays

## `BufferLayout` and `BufferElement`

[`src/renderer/Buffer.h`](../../OpenglGeometry/src/renderer/Buffer.h)

Describes the memory layout of a vertex so `VertexArray` can call `glVertexAttribPointer`
correctly.

```cpp
enum class ShaderDataType
{
    None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
};
```

`ShaderDataTypeSize(type)` gives the byte size; `BufferElement::GetComponentCount()` gives
the number of components.

```cpp
struct BufferElement
{
    std::string    Name;        // documentation only — attributes bind by index, not by name
    ShaderDataType Type;
    uint32_t       Size;        // bytes
    size_t         Offset;      // computed by BufferLayout
    bool           Normalized;
};
```

```cpp
BufferLayout layout =
{
    { ShaderDataType::Float4, "position" },
    { ShaderDataType::Float3, "color"    }
};
```

The constructor runs `CalculateOffsetsAndStride()`, filling each element's `Offset` and the
layout's `Stride` — a packed, interleaved, sequential layout. Attributes are assigned
locations **in declaration order**, so the layout above corresponds to:

```glsl
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;
```

The `Name` string is never sent to OpenGL. Renaming it changes nothing; reordering the
elements changes everything.

`Mat3`/`Mat4` report a component count of 3/4, which is not how matrix attributes actually
work in GL (they consume several consecutive locations). No current layout uses them.

### Layouts in use

| Layout | Used by |
| --- | --- |
| `{ Float4 "position" }` | Everything except the grid and cursor — torus, polylines, curves, surfaces, square, `StaticMeshType::Crosshair` (the selection-centre marker), cube. Also the default in `MeshGenerator::GeneratedMesh`. |
| `{ Float4 "position", Float2 "lineInfo" }` | `StaticMeshType::Grid`, matching `infiniteGrid.vert` |
| `{ Float4 "position", Float3 "color" }` | `StaticMeshType::Cursor` — per-axis colour, matching `cursor.vert` |
| `{ Float4 "a_Position" }` | Bézier curve and surface generation in `MeshGeneratingSystem` — same layout, different name string |

---

## `VertexBuffer`

[`VertexBuffer.h`](../../OpenglGeometry/src/renderer/VertexBuffer.h) ·
[`.cpp`](../../OpenglGeometry/src/renderer/VertexBuffer.cpp)

```cpp
VertexBuffer();                                // empty, no storage yet
VertexBuffer(uint32_t size);                   // GL_DYNAMIC_DRAW, uninitialised
VertexBuffer(float* vertices, uint32_t size);  // GL_STATIC_DRAW, initialised

void Bind() const;                             // GL_ARRAY_BUFFER
void Unbind() const;
void SetSubData(const void* data, uint32_t size);   // glBufferSubData, no reallocation
void SetData(const void* data, uint32_t size);      // glBufferData, reallocates

const BufferLayout& GetLayout() const;
void SetLayout(const BufferLayout& layout);
```

`size` is always in **bytes**, not element count.

`SetData` uses `GL_STATIC_DRAW` even though it is called every time geometry is regenerated
(`ModifyOrCreateMesh`). `GL_DYNAMIC_DRAW` would be the more accurate hint for frequently
re-uploaded meshes.

**Changing `SetLayout` after the buffer has been added to a `VertexArray` has no effect on
the VAO.** `VertexArray::AddVertexBuffer` reads the layout once and configures the attribute
pointers at that moment. `ModifyOrCreateMesh` calls `SetLayout` on its reuse path, which is
harmless only because the layout never actually changes for a given entity.

## `IndexBuffer`

[`IndexBuffer.h`](../../OpenglGeometry/src/renderer/IndexBuffer.h) ·
[`.cpp`](../../OpenglGeometry/src/renderer/IndexBuffer.cpp)

```cpp
IndexBuffer(uint32_t* indices, uint32_t count);
void Bind() const;                                    // GL_ELEMENT_ARRAY_BUFFER
void Unbind() const;
void SetIndices(uint32_t* indices, uint32_t count);
uint32_t GetCount() const;
```

Indices are always `uint32_t`, matching the hard-coded `GL_UNSIGNED_INT` in
`Renderer::Render`. `GetCount()` is the element count passed to `glDrawElements`.

> The constructor and `SetIndices` upload through **`GL_ARRAY_BUFFER`**, not
> `GL_ELEMENT_ARRAY_BUFFER`. The data lands in the right buffer object (the target is just a
> binding point), and `Bind()` correctly binds it as the element array, so rendering works —
> but the upload clobbers whatever was bound to `GL_ARRAY_BUFFER` at the time. See
> [gotchas](../gotchas.md).

## `VertexArray`

[`VertexArray.h`](../../OpenglGeometry/src/renderer/VertexArray.h) ·
[`.cpp`](../../OpenglGeometry/src/renderer/VertexArray.cpp)

Owns a VAO plus the buffers bound into it, so lifetime is handled by `Ref<VertexArray>` in
`MeshComponent`.

```cpp
class VertexArray
{
public:
    VertexArray();                        // glGenVertexArrays
    ~VertexArray();                       // glDeleteVertexArrays

    void Bind() const;
    void Unbind() const;

    void AddVertexBuffer(const Ref<VertexBuffer>&);
    void SetIndexBuffer(const Ref<IndexBuffer>&);

    const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const;
    const Ref<IndexBuffer>& GetIndexBuffer() const;

    template <typename T>
    static Ref<VertexArray> CreateWithBuffers(std::vector<T>& vertices,
                                              std::vector<uint32_t>& indices,
                                              const BufferLayout& layout);
private:
    uint32_t m_RendererID;
    uint32_t m_VertexBufferIndex = 0;     // running attribute location counter
    std::vector<Ref<VertexBuffer>> m_VertexBuffers;
    Ref<IndexBuffer> m_IndexBuffer;
};
```

### `AddVertexBuffer`

```cpp
glBindVertexArray(m_RendererID);
vertexBuffer->Bind();

for (const auto& element : vertexBuffer->GetLayout())
{
    glEnableVertexAttribArray(m_VertexBufferIndex);
    glVertexAttribPointer(m_VertexBufferIndex,
                          element.GetComponentCount(),
                          ShaderDataTypeToOpenGLBaseType(element.Type),
                          element.Normalized ? GL_TRUE : GL_FALSE,
                          layout.GetStride(),
                          (const void*)element.Offset);
    m_VertexBufferIndex++;
}
m_VertexBuffers.push_back(vertexBuffer);
```

`m_VertexBufferIndex` is a member, not a local, so attribute locations keep counting across
multiple vertex buffers: buffer 0 with two elements takes locations 0 and 1, and the first
element of buffer 1 gets location 2. Every mesh in the project uses a single interleaved
buffer, so in practice locations start at 0 and match the GLSL declarations.

### `CreateWithBuffers`

The one-call constructor used everywhere:

```cpp
template <typename T>
static Ref<VertexArray> CreateWithBuffers(std::vector<T>& vertices,
                                          std::vector<uint32_t>& indices,
                                          const BufferLayout& layout)
{
    auto result = CreateRef<VertexArray>();

    auto VBO = CreateRef<VertexBuffer>();
    VBO->SetData(vertices.data(), vertices.size() * sizeof(T));
    VBO->SetLayout(layout);
    result->AddVertexBuffer(VBO);

    auto IBO = CreateRef<IndexBuffer>(indices.data(), indices.size());
    result->SetIndexBuffer(IBO);

    return result;
}
```

`T` is `float` for the static meshes in `StaticMeshManager` (raw float arrays) and
`Algebra::Vector4` for generated geometry — both work because the layout, not `T`, describes
the memory.

Called from `StaticMeshManager`'s constructor and from
`MeshGeneratingSystem::ModifyOrCreateMesh` (creation path).

## Updating an existing mesh

The reuse path in `ModifyOrCreateMesh` avoids recreating GL objects:

```cpp
auto vertexArray = meshComponent.mesh;
vertexArray->GetVertexBuffers()[0]->SetData(vertices.data(), /* bytes */);
vertexArray->GetVertexBuffers()[0]->SetLayout(layout);
vertexArray->GetIndexBuffer()->SetIndices(indices.data(), indices.size());
```

This assumes exactly one vertex buffer and a non-null index buffer — true for every mesh
built through `CreateWithBuffers`.

## Lifetime

```
MeshComponent (Ref<VertexArray>)
   └─ VertexArray            glGenVertexArrays  / glDeleteVertexArrays
        ├─ Ref<VertexBuffer> glCreateBuffers    / glDeleteBuffers
        └─ Ref<IndexBuffer>  glCreateBuffers    / glDeleteBuffers
```

Destruction is automatic when the last `Ref` drops, which normally happens when
`RemovalSystem` destroys the entity. Meshes obtained from `StaticMeshManager` are shared and
outlive every entity, because the manager keeps a reference for the process lifetime.

> Both destructors call GL delete functions with no context check. Destroying a
> `VertexArray` after `glfwTerminate()` (i.e. after `~App`) is undefined behaviour. Keep all
> scene teardown before window teardown.
