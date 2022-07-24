cmake_minimum_required(VERSION 3.22)
project(ezfs C)

set(CMAKE_C_STANDARD 99)

add_executable(ezfs main.c src/dir_tree.c include/dir_tree.h src/ezfs_util.c include/ezfs_util.h src/ezfs.c)
