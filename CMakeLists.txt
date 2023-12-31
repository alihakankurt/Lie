cmake_minimum_required(VERSION 3.20)

project(Lie VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

add_executable(lie src/Lie.cc src/Buffer.cc src/Terminal.cc src/Editor.cc)

target_include_directories(lie PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
