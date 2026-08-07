project (Algebra)

add_library(${PROJECT_NAME}
	"src/Matrix4.cpp"
	"src/Quaternion.cpp"
	"src/Utils.cpp"
	"src/Vector4.cpp"
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_include_directories (${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/src
)
