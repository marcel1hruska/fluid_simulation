CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(glm_fetcher)
include(ExternalProject)
find_package(Git REQUIRED)
find_package(GLM QUIET)

if(GLM_FOUND)
    message(STATUS "Found GLM")
else()
    message(STATUS "GLM not found - will build from source")

	set(GLM_PREFIX ${CMAKE_BINARY_DIR}/externals/glm)

   ExternalProject_Add(
	glm
	PREFIX				${GLM_PREFIX}
	GIT_REPOSITORY		https://github.com/g-truc/glm.git
	GIT_TAG				1498e094b95d1d89164c6442c632d775a2a1bab5
	CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
	INSTALL_COMMAND		""
	)

	set(GLM_INCLUDE_DIRS ${GLM_PREFIX}/src/glm)
endif()

set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIRS} CACHE STRING "")