cmake_minimum_required(VERSION 2.8.4)
project(emui C)
include(appversion.cmake)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
find_package(emcrk REQUIRED)
find_package(emdas REQUIRED)
include_directories(SYSTEM ${CURSES_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/include)

add_definitions(-Wall -std=c99 -D_XOPEN_SOURCE=700 -DEMUI_VERSION="${APP_VERSION}")

if(DEBUG)
	add_definitions(-DDEBUG -g -O0)
else(DEBUG)
	add_definitions(-DNDEBUG -O3)
endif(DEBUG)

add_subdirectory(src)

configure_file(
	emui-config.cmake.in
	${PROJECT_BINARY_DIR}/emui-config.cmake
	@ONLY
)

configure_file(
	emui-config-version.cmake.in
	${PROJECT_BINARY_DIR}/emui-config-version.cmake
	@ONLY
)

install(FILES
	${PROJECT_BINARY_DIR}/emui-config.cmake
	${PROJECT_BINARY_DIR}/emui-config-version.cmake
	DESTINATION lib/emui
	COMPONENT dev
)

# vim: tabstop=4
