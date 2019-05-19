cmake_minimum_required (VERSION 3.8)
project (fluid_simulation)

find_package(OpenGL REQUIRED)

set (CMAKE_CXX_STANDARD 17)

configure_file(${CMAKE_SOURCE_DIR}/shaders/water_vertex_shader.vert ${CMAKE_BINARY_DIR}/shaders/water_vertex_shader.vert COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/water_fragment_shader.frag ${CMAKE_BINARY_DIR}/shaders/water_fragment_shader.frag COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/swe_compute_shader.comp ${CMAKE_BINARY_DIR}/shaders/swe_compute_shader.comp COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/update_compute_shader.comp ${CMAKE_BINARY_DIR}/shaders/update_compute_shader.comp COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/flow_compute_shader.comp ${CMAKE_BINARY_DIR}/shaders/flow_compute_shader.comp COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/HUD_vertex_shader.vert ${CMAKE_BINARY_DIR}/shaders/HUD_vertex_shader.vert COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/HUD_fragment_shader.frag ${CMAKE_BINARY_DIR}/shaders/HUD_fragment_shader.frag COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/crosshair_vertex_shader.vert ${CMAKE_BINARY_DIR}/shaders/crosshair_vertex_shader.vert COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/crosshair_fragment_shader.frag ${CMAKE_BINARY_DIR}/shaders/crosshair_fragment_shader.frag COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/terrain_vertex_shader.vert ${CMAKE_BINARY_DIR}/shaders/terrain_vertex_shader.vert COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/terrain_fragment_shader.frag ${CMAKE_BINARY_DIR}/shaders/terrain_fragment_shader.frag COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/resources/FreeSans.ttf ${CMAKE_BINARY_DIR}/resources/FreeSans.ttf COPYONLY)

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include( external_glew )
include_directories( ${GLEW_INCLUDE_DIRS} )
link_directories( ${GLEW_LIBS} )
	
include( external_glfw )
include_directories( ${GLFW_INCLUDE_DIRS} )
link_directories( ${GLFW_LIBS} )

include( external_freetype )
include_directories( ${FREETYPE_INCLUDE_DIRS} )
link_directories( ${FREETYPE_LIBS} )

include(external_glm)
include_directories( ${GLM_INCLUDE_DIRS} )

add_executable(fluid_simulation 
	main.cpp 
	utils/shader.cpp
	utils/camera.cpp
	utils/hud.cpp
	utils/perlin.cpp
	simulator.cpp
	motion.cpp)

add_dependencies(fluid_simulation glfw glew glm free_type)

target_link_libraries(fluid_simulation ${OPENGL_LIBRARY} glfw3 glew32s freetype)