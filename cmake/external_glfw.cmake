CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(glfw_fetcher)
include(ExternalProject)
find_package(GLFW QUIET)

if(GLFW_FOUND)
    message(STATUS "Found GLFW")
elseif(WIN32)
    message(STATUS "GLFW not found - will download Windows binaries")

	set(GLFW_PREFIX ${CMAKE_BINARY_DIR}/externals/glfw)

	ExternalProject_Add(
		glfw
		PREFIX				${GLFW_PREFIX}
		URL					https://github.com/glfw/glfw/releases/download/3.2.1/glfw-3.2.1.bin.WIN32.zip
		URL_MD5				c1fce22f39deab17a819da9d23b3a002
		CONFIGURE_COMMAND   ""
		BUILD_COMMAND       ""
		INSTALL_COMMAND		""
	)

	set(GLFW_INCLUDE_DIRS ${GLFW_PREFIX}/src/glfw/include)
	set(GLFW_LIBS ${GLFW_PREFIX}/src/glfw/lib-vc2015)
else()
	message(FATAL_ERROR "GLFW required")
endif()

set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIRS} CACHE STRING "")
set(GLFW_LIBS ${GLFW_LIBS} CACHE STRING "")