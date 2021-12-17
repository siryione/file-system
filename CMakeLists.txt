cmake_minimum_required(VERSION 3.16)

project(spz_cpp_2)

file(REMOVE "build/test_file")

add_executable(spz_2_all main.cpp)

execute_process(COMMAND "./spz_2_all")
