CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(glew_fetcher)
include(ExternalProject)
find_package(GLEW QUIET)

if(GLEW_FOUND)
    message(STATUS "Found GLEW")
elseif(WIN32)
    message(STATUS "GLEW not found - will download Windows binaries")
	set(GLEW_PREFIX ${CMAKE_BINARY_DIR}/externals/glew)

	ExternalProject_Add(
		glew
		PREFIX				${GLEW_PREFIX}
		URL					https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip
		URL_MD5				32a72e6b43367db8dbea6010cd095355
		CONFIGURE_COMMAND   ""
		BUILD_COMMAND       ""
		INSTALL_COMMAND		""
	)

	set(GLEW_INCLUDE_DIRS ${GLEW_PREFIX}/src/glew/include)
	set(GLEW_LIBS ${GLEW_PREFIX}/src/glew/lib/Release/Win32)
else()
	message(FATAL_ERROR "GLEW required")
endif()

add_definitions(
	-DGLEW_STATIC
)

set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIRS} CACHE STRING "")
set(GLEW_LIBS ${GLEW_LIBS} CACHE STRING "")