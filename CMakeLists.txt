#Copyright 2017 Lukasz Towarek
#
#Licensed under the Apache License, Version 2.0 (the "License");
#you may not use this file except in compliance with the License.
#You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#Unless required by applicable law or agreed to in writing, software
#distributed under the License is distributed on an "AS IS" BASIS,
#WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#See the License for the specific language governing permissions and
#limitations under the License.

project(vulkanalia)
cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(cmake/gtest.cmake.in ${CMAKE_BINARY_DIR}/download/gtest/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . 
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/download/gtest")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . 
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/download/gtest")

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_BINARY_DIR}/external/googletest-src"
                 "${CMAKE_BINARY_DIR}/external/googletest-build")
add_library(GTest::GTest ALIAS gtest)
add_library(GTest::Main ALIAS gtest_main)

configure_file(cmake/glfw.cmake.in ${CMAKE_BINARY_DIR}/download/glfw/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . 
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/download/glfw")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . 
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/download/glfw")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)  
add_subdirectory("${CMAKE_BINARY_DIR}/external/glfw-src"
                 "${CMAKE_BINARY_DIR}/external/glfw-build")
add_library(GLFW::GLFW ALIAS glfw)

configure_file(cmake/glm.cmake.in ${CMAKE_BINARY_DIR}/download/glm/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . 
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/download/glm")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . 
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/download/glm")

add_subdirectory("${CMAKE_BINARY_DIR}/external/glm-src"
                 "${CMAKE_BINARY_DIR}/external/glm-build")
add_library(GLM::GLM ALIAS glm)

configure_file(cmake/stb.cmake.in ${CMAKE_BINARY_DIR}/download/stb/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . 
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/download/stb")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . 
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/download/stb")

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_BINARY_DIR}/external/stb-src")
add_library(STB::STB ALIAS stb)

configure_file(cmake/tinyobjloader.cmake.in ${CMAKE_BINARY_DIR}/download/tinyobjloader/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . 
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/download/tinyobjloader")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . 
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/download/tinyobjloader")

add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE "${CMAKE_BINARY_DIR}/external/tinyobjloader-src")
add_library(TINYOBJLOADER::TINYOBJLOADER ALIAS tinyobjloader)

find_package(Vulkan REQUIRED)

add_library(triangle triangle.hpp triangle.cpp)
add_library(vka::triangle ALIAS triangle)
target_link_libraries(triangle GLFW::GLFW Vulkan::Vulkan GLM::GLM STB::STB TINYOBJLOADER::TINYOBJLOADER)

add_executable(vulkanalia main.cpp)
target_link_libraries(vulkanalia vka::triangle)

add_custom_command(TARGET vulkanalia POST_BUILD 
                   COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/vert.spv"              
                   $<TARGET_FILE_DIR:vulkanalia>)
add_custom_command(TARGET vulkanalia POST_BUILD 
                   COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/frag.spv"              
                   $<TARGET_FILE_DIR:vulkanalia>)
add_custom_command(TARGET vulkanalia POST_BUILD 
                   COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/texture.jpg"              
                   $<TARGET_FILE_DIR:vulkanalia>)

add_executable(vulkanalia_test test.cpp)
target_link_libraries(vulkanalia_test PRIVATE GTest::GTest GTest::Main vka::triangle)

add_custom_command(TARGET vulkanalia_test POST_BUILD 
                   COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/vert.spv"              
                   $<TARGET_FILE_DIR:vulkanalia_test>)
add_custom_command(TARGET vulkanalia_test POST_BUILD 
                   COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/texture.jpg"              
                   $<TARGET_FILE_DIR:vulkanalia_test>)
