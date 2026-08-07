# Managers

[`src/managers/`](../OpenglGeometry/src/managers/) — three Meyers singletons owning
process-lifetime resources.

All three follow the same shape:

```cpp
class XManager
{
private:
    XManager();                             // private ctor, does the loading
public:
    ~XManager() = default;
    XManager(XManager& other) = delete;
    void operator=(const XManager&) = delete;

    static XManager& GetInstance();         // static local → thread-safe, lazy
};
```

`GetInstance()` returns a reference to a function-local `static`, so construction happens on
first use and destruction at program exit, in reverse order of construction.

> **Initialisation order matters.** `ShaderManager` and `StaticMeshManager` make OpenGL calls
> in their constructors, so the first `GetInstance()` must happen *after* the GL context
> exists. It does today: the first calls come from `BaseScene` and `RenderingSystem`, both
> constructed after `Window`. Do not call them from a static initialiser.
>
> Destruction at exit is the mirror problem: these singletons hold GL objects that are
> released after `glfwTerminate()` runs in `~App`. See [gotchas](gotchas.md).

---

## `ShaderManager`

[`ShaderManager.h`](../OpenglGeometry/src/managers/ShaderManager.h) ·
[`.cpp`](../OpenglGeometry/src/managers/ShaderManager.cpp)

Owns one compiled `Shader` per entry of the `AvailableShaders` enum.

```cpp
enum class AvailableShaders
{
    Default                 = 0,
    InfiniteGrid            = 1,
    BezierCurveC0           = 2,
    BezierSurfaceHorizontal = 3,
    BezierSurfaceVertical   = 4
};
```

All programs are built eagerly in the constructor:

```cpp
ShaderManager::ShaderManager()
{
    std::clog << "INFO: Loading Shaders" << std::endl;

    AssignShader(AvailableShaders::Default, ShaderBuilder("resources/shaders/")
        .AddShader(ShaderType::Vertex,   "default")
        .AddShader(ShaderType::Fragment, "default"));

    AssignShader(AvailableShaders::InfiniteGrid, ShaderBuilder("resources/shaders/")
        .AddShader(ShaderType::Vertex,   "infiniteGrid")
        .AddShader(ShaderType::Fragment, "infiniteGrid"));

    AssignShader(AvailableShaders::BezierCurveC0, ShaderBuilder("resources/shaders/bezierLine/")
        .AddShader(ShaderType::Vertex,                "default")
        .AddShader(ShaderType::TesselationControl,    "default")
        .AddShader(ShaderType::TesselationEvaluation, "default")
        .AddShader(ShaderType::Fragment,              "default")
        .ChangePatchSize(4));

    AssignShader(AvailableShaders::BezierSurfaceHorizontal, /* ... */ .ChangePatchSize(16));
    AssignShader(AvailableShaders::BezierSurfaceVertical,   /* ... */ .ChangePatchSize(16));

    std::clog << "INFO: Loaded Shaders" << std::endl;
}
```

`AssignShader` warns and returns the existing program if the enum value is already taken.

```cpp
std::shared_ptr<Shader> GetShader(AvailableShaders name);
```

Returns `shaders[name]` from an `unordered_map` — **note this is `operator[]`**, so asking
for an unregistered enum value default-constructs a null `shared_ptr` rather than failing
loudly. `Renderer::Render` guards with `if (!m_ActualShader) return;`, so the symptom is
silently invisible geometry.

There is no hot-reload. Editing GLSL requires a rebuild (which re-runs the resource copy
step) and a restart.

To add a program, see [how-to/add-a-new-shader.md](how-to/add-a-new-shader.md).

---

## `StaticMeshManager`

[`StaticMeshManager.h`](../OpenglGeometry/src/managers/StaticMeshManager.h) ·
[`.cpp`](../OpenglGeometry/src/managers/StaticMeshManager.cpp)

Owns the meshes that are shared rather than generated per-entity.

```cpp
enum class StaticMeshType { Cube, Square, Cursor, Grid };

Ref<VertexArray> GetMesh(StaticMeshType type);
```

Vertex data is hard-coded in the constructor (there is a `//TODO: Make it loaded from files`),
and load time is measured and logged.

| Mesh | Geometry | Layout | Used by |
| --- | --- | --- | --- |
| `Square` | 0.1 × 0.1 quad in XY, 2 triangles | `{ Float4 }` | Every point entity |
| `Cube` | Unit cube, 12 triangles | `{ Float4 }` | Nothing currently |
| `Cursor` | 3 lines from the origin along +X, +Y, +Z (0.2 long) | `{ Float4 }` | The 3D cursor |
| `Grid` | 201×201 lines over [-1, 1] with per-vertex colour | `{ Float4, Float3 }` | The infinite grid |

The grid's colours encode its structure: grey by default, lighter grey every 10th line, red
on the centre line. The `infiniteGrid` shader then scales and translates it to look infinite —
see [renderer/shader-reference.md](renderer/shader-reference.md#infinitegrid).

Because these `Ref<VertexArray>`s are held by the manager for the process lifetime, entities
sharing them can be destroyed freely without the GPU buffers going away. That is the whole
point: creating 500 points allocates no new GL objects.

`GetMesh` uses `operator[]` too, so an unregistered type yields a null `Ref` that will crash
in `Renderer::Render` (which does not null-check the mesh).

---

## `IdManager`

[`IdManager.h`](../OpenglGeometry/src/managers/IdManager.h) ·
[`.cpp`](../OpenglGeometry/src/managers/IdManager.cpp)

Hands out and recycles the ids used in shape names.

```cpp
using ID = long long;

ID   GetNewId();      // pop a freed id, or bump the counter
void FreeId(ID id);   // push onto the free queue
```

```cpp
ID IdManager::GetNewId()
{
    if (m_FreeIds.empty())
        return m_CurrentFreeId++;

    ID result = m_FreeIds.front();
    m_FreeIds.pop();
    return result;
}
```

Used only by `IdComponent`, whose constructor takes an id and whose destructor returns it.
So deleting "Torus 3" and creating a new torus reuses the number 3.

This id is **not** the entity handle. `Entity::GetID()` returns the raw EnTT index, which is
what ImGui labels use for uniqueness; `IdComponent::id` is what appears in display names.

Reuse is FIFO, so ids come back in the order they were freed rather than lowest-first.
