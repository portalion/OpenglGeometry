# Core and application layer

Covers [`src/App.*`](../OpenglGeometry/src/App.cpp), [`src/core/`](../OpenglGeometry/src/core/),
[`src/interfaces/`](../OpenglGeometry/src/interfaces/) and
[`src/utils/`](../OpenglGeometry/src/utils/).

---

## `main`

[`src/main.cpp`](../OpenglGeometry/src/main.cpp)

```cpp
int main(void)
{
    if (!glfwInit())
        return -1;

    App app;
    app.Run();

    return 0;
}
```

GLFW is initialised before `App`, because `Window`'s constructor creates the GLFW window.
`glfwTerminate()` happens in `~App`.

---

## `App`

[`App.h`](../OpenglGeometry/src/App.h) · [`App.cpp`](../OpenglGeometry/src/App.cpp)

```cpp
class App
{
public:
    App();
    ~App();

    void Run();
    void HandleResize();
    Algebra::Vector4 ScreenToNDC(float x, float y);
    void GetClickedPoint();
    // Render/HandleInput/Update/DisplayParameters/CreateShape — declared, not defined
private:
    bool running;
    bool showGrid = true;
    Window window;
    Unique<SystemPipeline> systemPipeline;
    Ref<Scene> currentScene;
};
```

### Construction

```cpp
App::App()
    : window{ Globals::startingSceneWidth + Globals::rightInterfaceWidth,
              Globals::startingSceneHeight, "Geometry" },
      running{ true }
{
    InitImgui(window.GetWindowPointer());
    ImGui::StyleColorsDark();
    window.SetAppPointerData(this);
    HandleResize();

    currentScene   = CreateRef<BaseScene>();
    systemPipeline = CreateUnique<SystemPipeline>(currentScene);
}
```

Order is load-bearing: the window (and therefore the GL context) exists before ImGui is
initialised and before the scene is built — and the scene's constructor is what first touches
`StaticMeshManager`, which issues GL calls.

The window is created wider than the render area by `rightInterfaceWidth` (400 px), reserving
space for the ImGui panels.

### `Run`

The frame loop, documented in [architecture.md](architecture.md#the-frame-loop).

### Destruction

```cpp
App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}
```

Note the singletons (`ShaderManager`, `StaticMeshManager`) are destroyed *after* this, at
static destruction time, when there is no longer a GL context. See [gotchas](gotchas.md).

### Vestigial members

`Render`, `HandleInput`, `Update`, `DisplayParameters`, `CreateShape` are **declared but never
defined** — leftovers from the pre-ECS architecture. `HandleResize` computes an aspect ratio
and discards it; `ScreenToNDC` and `GetClickedPoint` are half-finished picking code with no
caller. `showGrid` is never read.

These are the natural hooks if you add viewport picking or resize-aware projection, but today
they do nothing.

---

## `core/Base.h`

```cpp
template<typename T> using Ref = std::shared_ptr<T>;
template<typename T, typename... Args> constexpr Ref<T> CreateRef(Args&&... args);

template<typename T> using Unique = std::unique_ptr<T>;
template<typename T, typename... Args> constexpr Unique<T> CreateUnique(Args&&... args);
```

Used throughout in preference to the standard names. `Ref` for anything shared (scenes,
systems, shaders, meshes), `Unique` for exclusive ownership (`App::systemPipeline`).

## `core/Globals.h`

```cpp
struct Globals
{
    static const int startingSceneWidth;    // 1280
    static const int startingSceneHeight;   //  960
    static const int rightInterfaceWidth;   //  400

    static const float cameraNearPlane;     // 0.1
    static const float cameraFarPlane;      // 1000
    static const float cameraFieldOfView;   // pi/2

    static const Algebra::Vector4 startingCameraTarget;   // (0, 0, 0, 1)
    static const float            startingCameraDistance; // 10
    static const float            startingCameraPitch;    // 0.3

    static const Algebra::Vector4 defaultPointsColor;     // (1, 0.2, 0, 1)   - EntityContext default u_color
    static const Algebra::Vector4 selectionColor;         // (1, 0.85, 0.15, 1) - selection yellow
};
```

Compile-time configuration constants, defined in `Globals.cpp`. The camera frustum values feed
`Matrix4::Projection` in `RenderingSystem` / `BaseScene`; the starting target/distance/pitch
seed the `OrbitCamera`. `defaultPointsColor` is the `EntityContext::Color` default;
`selectionColor` is used both by `RenderingSystem` (which swaps it in for any entity carrying
`IsSelectedTag`) and as the `ColorComponent` of the selection-centre crosshair.

## `core/Window.h`

```cpp
struct WindowUserPointerData
{
    class App* app;
    class Window* window;
};

class Window
{
public:
    Window(int width, int height, std::string title);

    int GetWidth();  int GetHeight();
    std::string GetTitle();
    GLFWwindow* GetWindowPointer();

    bool ShouldClose();
    void ProcessFrame();                       // swap buffers + poll events
    void HandleResize(int width, int height);
    void SetAppPointerData(App* app);
private:
    void SetupGLFWFunctions();
};
```

The constructor requests an OpenGL **4.6 core profile** context, creates the window, stores a
`WindowUserPointerData` as the GLFW user pointer, makes the context current, initialises GLEW
and enables blending.

The framebuffer-size callback goes through the user pointer to reach both objects:

```cpp
glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* window, int w, int h) {
    auto* windowData = static_cast<WindowUserPointerData*>(glfwGetWindowUserPointer(window));
    windowData->window->HandleResize(w, h);
    windowData->app->HandleResize();
});
```

`Window::HandleResize` sets the viewport, reserving the UI strip:

```cpp
GLCall(glViewport(0, 0, std::max(width - Globals::rightInterfaceWidth, 0), height));
```

`App::HandleResize` is where the camera's projection matrix *should* be updated but is not —
so resizing the window stretches the scene. See [gotchas](gotchas.md).

> `data.app` is only set by `SetAppPointerData`, called from `App`'s constructor. The
> constructor's own `HandleResize(width, height)` call happens before the callback is
> installed, so the uninitialised `app` pointer is never dereferenced — but the ordering is
> fragile.

---

## `ICamera` / `OrbitCamera`

[`interfaces/ICamera.h`](../OpenglGeometry/src/interfaces/ICamera.h)

```cpp
class ICamera
{
public:
    virtual ~ICamera() = default;

    virtual void HandleInput(CameraComponent&, const ViewportData&) = 0;

    virtual Algebra::Vector4 GetPosition() const = 0;
    virtual Algebra::Vector4 GetTarget()   const = 0;
    virtual Algebra::Vector4 GetRight()    const = 0;
    virtual Algebra::Vector4 GetUp()       const = 0;
    virtual Algebra::Vector4 GetForward()  const = 0;
    virtual void Focus(const Algebra::Vector4& point, float radius) = 0;
};
```

`HandleInput` reads ImGui IO and writes `cameraComponent.viewMatrix`.
`CameraComponent::cameraHandling` holds a `Ref<ICamera>`, so camera behaviour is swappable
per entity without touching `RenderingSystem`. The `GetPosition/Right/Up/Forward` accessors
give viewport picking and cursor placement a world-space camera basis for building rays
(`src/ui/ViewportMath.cpp`).

### `OrbitCamera`

[`OrbitCamera.h`](../OpenglGeometry/src/core/OrbitCamera.h) ·
[`.cpp`](../OpenglGeometry/src/core/OrbitCamera.cpp)

Orbits a `target` point at a given `distance`, with Euler `yaw` / `pitch` / `roll`:

```cpp
Algebra::Matrix4 GetViewMatrix() const
{
    return Matrix4::Translation(0, 0, -distance)
         * GetRotationMatrix()                       // RotZ(roll) * RotX(pitch) * RotY(yaw)
         * Matrix4::Translation(-target.x, -target.y, -target.z);
}
```

| Input | Handler | Behaviour |
| --- | --- | --- |
| Middle-drag | `HandlePan` | Moves `target` along the camera right/up axes; speed scales with `distance` and FOV so it tracks the cursor |
| Wheel | `HandleZoom` | `distance *= 0.9^wheel`, clamped to `[0.5, 500]` |
| Right-drag | `HandleRotations` | `yaw`/`pitch` from the drag delta over viewport width/height; pitch clamped to `±1.56` rad |
| Shift + right-drag | `HandleRotations` | `roll` from the horizontal delta |

`GetPosition()` is `target + Rᵀ·(0,0,distance)`; `Focus(point, radius)` recentres `target`
and pulls `distance` in to frame a sphere of that radius. Input comes from ImGui
(`ImGui::IsMouseDragging`, `ImGui::GetIO()`), so a right-drag that started over a panel still
counts — and a right *click* that never crosses the drag threshold does nothing, which is what
frees it for [cursor placement](systems/gui-systems.md#viewport-point-picking).

---

## `Notifier` / `IObserver` — dead code

[`core/Notifier.h`](../OpenglGeometry/src/core/Notifier.h),
[`interfaces/IObserver.h`](../OpenglGeometry/src/interfaces/IObserver.h)

A classic observer pattern (`AddObserver` / `RemoveObserver` / `Notify`). **Nothing in the
project uses either type.** They were superseded by the ECS notification mechanism
(`Observable` + `NotificationComponent` + `NotificationSystem`) documented in
[ecs/change-propagation.md](ecs/change-propagation.md).

Do not build on them. `Notifier.cpp` is still compiled into the binary.

---

## `utils/`

### `Initialization.{h,cpp}`

```cpp
bool InitImgui(GLFWwindow* window);   // context, keyboard/gamepad nav, docking, GLFW+GL3 backends
bool InitGLEW();                      // glewInit + log GL/GLSL versions
```

`Initialization.h` is also the project's umbrella header for ImGui and GL includes — several
files include it just for that.

Docking is enabled (`ImGuiConfigFlags_DockingEnable`), so the vendored ImGui is the docking
branch. The layout persists in `imgui.ini`, which is gitignored.

### `GlCall.{h,cpp}`

```cpp
#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLClearError(); x; ASSERT(GLLogCall(#x, __FILE__, __LINE__));
```

Clear the error queue, run the call, report and break on failure. Used in `Shader`, `Window`
and the buffer classes.

Caveats: it is active in release builds too, `__debugbreak()` is MSVC-only, and the macro
expands to multiple statements — never use it as an unbraced `if` body.

---

## `deprecated/`

[`OpenglGeometry/deprecated/ellipsoid/`](../OpenglGeometry/deprecated/ellipsoid/) contains
`RaycastableEllipsoid` and `Raycaster` from an earlier CPU raycasting version of the project.
**These files are not listed in `OpenglGeometry/CMakeLists.txt` and are not compiled.** They
reference headers that
no longer exist at those paths (`"Shader.h"`, `"Window.h"`) and would not build as-is.
