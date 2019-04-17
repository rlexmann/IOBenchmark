cmake_minimum_required(VERSION 3.6)

project (ioBenchmark)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES
	"${SOURCE_DIR}/*.cpp"
	"${SOURCE_DIR}/*.hpp"
)

find_package (Threads)

add_executable (ioBenchmark ${SOURCES})
target_include_directories (ioBenchmark PRIVATE ${INCLUDE_DIR})
target_link_libraries (ioBenchmark ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(ioBenchmark PROPERTIES SUFFIX ".exe")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ioBenchmark)