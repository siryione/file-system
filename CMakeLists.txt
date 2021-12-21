cmake_minimum_required(VERSION 3.16)

project(spz_cpp_2)

file(REMOVE "build/test_file")

file(GLOB project_src "*.hpp" "*.cpp")

add_executable(spz_2_all ${project_src} )

execute_process(COMMAND "./spz_2_all")
