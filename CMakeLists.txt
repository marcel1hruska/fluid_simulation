cmake_minimum_required (VERSION 3.8)
project (fluid_simulation)

find_package(OpenGL REQUIRED)

set (CMAKE_CXX_STANDARD 17)

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
	utils/loaders.cpp
	utils/camera.cpp
	utils/hud.cpp
	utils/perlin.cpp
	simulator.cpp
	motion.cpp)

add_dependencies(fluid_simulation glfw glew glm free_type)

target_link_libraries(fluid_simulation ${OPENGL_LIBRARY} glfw3 glew32s freetype)

add_custom_command(TARGET fluid_simulation PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
					COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders)