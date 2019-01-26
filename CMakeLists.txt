cmake_minimum_required(VERSION 3.8)
project(soft_body_sim)
set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES src/main.cpp
        src/Physics.hpp
        src/Shapes.hpp
        src/Object.hpp
        src/Utils.hpp)

include_directories(eigen3)

add_executable(soft_body_sim ${SOURCE_FILES})

target_link_libraries(soft_body_sim GLU GL glut)
