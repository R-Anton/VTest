# CMakeList.txt : CMake project for VTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.11)
set(CMAKE_PREFIX_PATH ${glfw3_DIR})

set(CMAKE_FIND_DEBUG_MODE True)
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
# Add source to this project's executable.
add_executable (VTest 
	"VTest.cpp" "VTest.h"
	vert.spv frag.spv
	)

add_custom_command(
	OUTPUT vert.spv
	COMMAND ${Vulkan_GLSLC_EXECUTABLE} -fshader-stage=vertex ${CMAKE_CURRENT_SOURCE_DIR}/vert.glsl  -o vert.spv
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vert.glsl
)

add_custom_command(
	OUTPUT frag.spv
	COMMAND ${Vulkan_GLSLC_EXECUTABLE} -fshader-stage=fragment ${CMAKE_CURRENT_SOURCE_DIR}/frag.glsl -o frag.spv
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/frag.glsl
)

#Vulkan_GLSLC_EXECUTABLE

set_target_properties(VTest
	PROPERTIES
	CXX_STANDARD 17
)

target_link_libraries(VTest
PRIVATE
	Vulkan::Vulkan
	glfw
)
# TODO: Add tests and install targets if needed.
