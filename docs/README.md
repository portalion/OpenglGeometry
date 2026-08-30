# OpenglGeometry — Documentation

OpenglGeometry is a Windows/Visual Studio OpenGL 4.6 application for interactively building
and viewing parametric geometry: points, tori, polylines, Bézier curves (C0, C2/B-spline,
interpolated) and Bézier surfaces (flat and cylindrical).

It is built on an **entity–component–system (ECS)** core (EnTT), a thin OpenGL abstraction
layer, and Dear ImGui for the interface. Geometry is generated on the CPU into vertex
buffers, and curves/surfaces are evaluated on the GPU with tessellation shaders.

---

## Start here

| Doc | What it covers |
| --- | --- |
| [Getting started](getting-started.md) | Prerequisites, how to build, how to run, repository layout |
| [Architecture](architecture.md) | The big picture: frame loop, data flow, why the code is split the way it is |
| [Build system](build-system.md) | How the CMake setup is put together and why |
| [Glossary](glossary.md) | Terms used throughout the codebase (archetype, virtual entity, dirty, patch, …) |

## Reference by subsystem

| Area | Docs |
| --- | --- |
| **ECS core** | [Overview](ecs/README.md) · [Components](ecs/components.md) · [Tags](ecs/tags.md) · [Change propagation](ecs/change-propagation.md) · [ComponentFunctionRegistry](ecs/component-function-registry.md) |
| **Systems** | [Overview & pipeline](systems/README.md) · [NotificationSystem](systems/notification-system.md) · [RemovalSystem](systems/removal-system.md) · [MeshGeneratingSystem](systems/mesh-generating-system.md) · [RenderingSystem](systems/rendering-system.md) · [GUI systems](systems/gui-systems.md) |
| **Rendering** | [Overview](renderer/README.md) · [Buffers & vertex arrays](renderer/buffers.md) · [Shader pipeline](renderer/shaders.md) · [Shader reference](renderer/shader-reference.md) |
| **Geometry** | [Overview](geometry/README.md) · [Bézier curves](geometry/bezier-curves.md) · [Bézier surfaces](geometry/bezier-surfaces.md) |
| **Building blocks** | [Archetypes](archetypes.md) · [Managers](managers.md) · [Core](core.md) · [UI](ui.md) · [Serialization](serialization.md) · [Algebra library](algebra.md) |

## Task recipes

Practical, step-by-step guides for the changes you are most likely to make:

- [Add a new shape](how-to/add-a-new-shape.md)
- [Add a new component](how-to/add-a-new-component.md)
- [Add a new system](how-to/add-a-new-system.md)
- [Add a new shader](how-to/add-a-new-shader.md)
- [Add a new file to the build](how-to/add-a-file-to-the-build.md)
- [Add a new inspector field](how-to/add-an-inspector-field.md)

## Also worth reading

- [Known issues and gotchas](gotchas.md) — bugs, sharp edges and dead code found while
  documenting. Read this before you spend an hour debugging something already known.

---

## One-paragraph summary of how it works

`main` creates an [`App`](../OpenglGeometry/src/App.cpp), which owns a [`Window`](../OpenglGeometry/src/core/Window.cpp)
(GLFW + GLEW + ImGui), a [`Scene`](../OpenglGeometry/src/scene/Scene.h) (an EnTT registry) and a
[`SystemPipeline`](../OpenglGeometry/src/systems/SystemPipeline.cpp). Every frame the pipeline runs its
systems in a fixed order: propagate change notifications → destroy entities marked for
removal → draw the ImGui panels (which is also where user edits happen) → regenerate meshes
for entities marked dirty → render everything with a `MeshComponent`. Shapes are not classes;
they are entities assembled from components by helper functions in
[`archetypes/`](../OpenglGeometry/src/archetypes/).
