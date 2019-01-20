cmake_minimum_required(VERSION 3.8)
project(projekt)
set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)

include_directories(eigen3)

add_executable(projekt ${SOURCE_FILES})

target_link_libraries(projekt GLU GL glut)
