cmake_minimum_required(VERSION 3.17.0)
project(gol)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set(COMPILE_FLAGS)
set(INCLUDE_DIRS)
set(LINK_LIBS)

add_executable(gol "")
add_subdirectory(src)
add_subdirectory(third_party/raylib)

list(APPEND COMPILE_FLAGS -Wall -Wextra -pedantic)

if (CMAKE_BUILD_TYPE MATCHES Release)
	list(APPEND COMPILE_FLAGS -03)
endif()

list(APPEND INCLUDE_DIRS
	"include"
	"third_party/raylib/src"
	"third_party/Nuklear")

list(APPEND LINK_LIBS raylib)

target_compile_options(gol PRIVATE ${COMPILE_FLAGS})
target_include_directories(gol PRIVATE SYSTEM ${INCLUDE_DIRS})
target_link_libraries(gol PRIVATE ${LINK_LIBS})
