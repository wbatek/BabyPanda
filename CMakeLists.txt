cmake_minimum_required(VERSION 3.10.2)
project(AbstractProgrammingProject)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")

string(APPEND CMAKE_CXX_FLAGS " -Wall")
string(APPEND CMAKE_CXX_FLAGS " -Wbuiltin-macro-redefined")
string(APPEND CMAKE_CXX_FLAGS " -pedantic")
string(APPEND CMAKE_CXX_FLAGS " -Werror")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.cpp")

#add_subdirectory(csv_parser)

add_executable(column
        src/column.cpp)
add_executable(dataframe
        src/dataframe.cpp)

#target_link_libraries(column csv)
#target_link_libraries(dataframe csv)

target_include_directories(column PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(dataframe PRIVATE ${PROJECT_SOURCE_DIR}/include)
