CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(freetype_fetcher)
include(ExternalProject)
find_package(Git REQUIRED)
find_package(FREETYPE QUIET)

if(FREETYPE_FOUND)
    message(STATUS "Found FREETYPE")
elseif(WIN32)
    message(STATUS "FREETYPE not found - will download WINDOWS BINARIES")

	set(FREETYPE_PREFIX ${CMAKE_BINARY_DIR}/externals/freetype)

   ExternalProject_Add(
	free_type
	PREFIX				${FREETYPE_PREFIX}
	GIT_REPOSITORY		https://github.com/ubawurinna/freetype-windows-binaries.git
	GIT_TAG				5e7caec39a431b402c32667e615a83681a217263
	CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
	INSTALL_COMMAND		${CMAKE_COMMAND} -E copy ${FREETYPE_PREFIX}/src/free_type/win32/freetype.dll ${CMAKE_BINARY_DIR}
	)
	set(FREETYPE_INCLUDE_DIRS ${FREETYPE_PREFIX}/src/free_type/include)
	set(FREETYPE_LIBS ${FREETYPE_PREFIX}/src/free_type/win32)
else()
	message(FATAL_ERROR "FREETYPE required")
endif()

set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS} CACHE STRING "")
set(FREETYPE_LIBS ${FREETYPE_LIBS} CACHE STRING "")