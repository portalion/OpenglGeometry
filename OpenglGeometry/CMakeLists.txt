project (OpenglGeometry)

add_executable(${PROJECT_NAME}
    "src/main.cpp"
    "src/App.cpp"

    "src/core/DragCamera.cpp"
    "src/core/Globals.cpp"
    "src/core/Notifier.cpp"
    "src/core/Window.cpp"

    "src/managers/IdManager.cpp"
    "src/managers/ShaderManager.cpp"
    "src/managers/StaticMeshManager.cpp"

    "src/meshGenerators/BezierCurveGenerator.cpp"
    "src/meshGenerators/InterpolatedBezierCurveGenerator.cpp"
    "src/meshGenerators/PolylineMeshGenerator.cpp"
    "src/meshGenerators/TorusMeshGenerator.cpp"

    "src/renderer/IndexBuffer.cpp"
    "src/renderer/Renderer.cpp"
    "src/renderer/Shader.cpp"
    "src/renderer/ShaderBuilder.cpp"
    "src/renderer/VertexArray.cpp"
    "src/renderer/VertexBuffer.cpp"

    "src/scene/BaseScene.cpp"
    "src/scene/Entity.cpp"
    "src/scene/Scene.cpp"

    "src/systems/MeshGeneratingSystem.cpp"
    "src/systems/NotificationSystem.cpp"
    "src/systems/RemovalSystem.cpp"
    "src/systems/RenderingSystem.cpp"
    "src/systems/SystemPipeline.cpp"
    "src/renderer/UniformApplier.cpp"

    "src/systems/gui/GUISystem.cpp"
    "src/systems/gui/PopupSystem.cpp"
    "src/systems/gui/ShapeInspectorSystem.cpp"

    "src/UI/popups/ShapeCreation.cpp"

    "src/utils/GlCall.cpp"
    "src/utils/Initialization.cpp")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_include_directories(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/src
)

target_include_directories (${PROJECT_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/Dependencies/glfw/include
	${CMAKE_SOURCE_DIR}/Dependencies/glew/include
)

target_link_libraries(${PROJECT_NAME}
	glfw
	libglew_static
	imgui
	entt
	Algebra
)

add_custom_target(copy_resources ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_SOURCE_DIR}/resources
	${PROJECT_BINARY_DIR}/resources
	COMMENT "Copying resources into binary directory")

add_dependencies(${PROJECT_NAME} copy_resources)
