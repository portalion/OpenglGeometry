project(imgui)

add_library(imgui)

target_include_directories(
	imgui PUBLIC
	${PROJECT_SOURCE_DIR}
)

target_sources(
	imgui PRIVATE
	${PROJECT_NAME}/imgui.cpp
	${PROJECT_NAME}/imgui_draw.cpp
	${PROJECT_NAME}/imgui_tables.cpp
	${PROJECT_NAME}/imgui_widgets.cpp
	${PROJECT_NAME}/imgui_demo.cpp
	${PROJECT_NAME}/imgui_stdlib.cpp
	${PROJECT_NAME}/imgui_impl_glfw.cpp
	${PROJECT_NAME}/imgui_impl_opengl3.cpp
)

target_link_libraries(imgui PUBLIC glfw libglew_static)

# EnTT is a single header, so it only needs an include path.
add_library(entt INTERFACE)

target_include_directories(
	entt INTERFACE
	${PROJECT_SOURCE_DIR}
)
