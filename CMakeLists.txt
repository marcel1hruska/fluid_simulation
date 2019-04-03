cmake_minimum_required (VERSION 3.8)
project (fluid_simulation)

find_package(OpenGL REQUIRED)

set (CMAKE_CXX_STANDARD 17)

configure_file(${CMAKE_SOURCE_DIR}/shaders/standard_vertex_shader.vs ${CMAKE_BINARY_DIR}/shaders COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/standard_fragment_shader.fs ${CMAKE_BINARY_DIR}/shaders COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/HUD_vertex_shader.vs ${CMAKE_BINARY_DIR}/shaders COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/HUD_fragment_shader.fs ${CMAKE_BINARY_DIR}/shaders COPYONLY)

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include( external_glew )
include_directories( ${GLEW_INCLUDE_DIRS} )
link_directories( ${GLEW_LIBS} )
	
include( external_glfw )
include_directories( ${GLFW_INCLUDE_DIRS} )
link_directories( ${GLFW_LIBS} )

include(external_glm)
include_directories( ${GLM_INCLUDE_DIRS} )

add_executable(fluid_simulation 
	main.cpp 
	utils/shader.cpp
	utils/camera.cpp
	utils/hud.cpp
	fluid_simulator.cpp)

add_dependencies(fluid_simulation glfw glew glm)

target_link_libraries(fluid_simulation ${OPENGL_LIBRARY} glfw3 glew32s)