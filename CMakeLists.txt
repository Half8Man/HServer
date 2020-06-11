cmake_minimum_required(VERSION 3.16)
project(HServer)

set(CMAKE_CXX_STANDARD 14)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

find_package(Threads)
add_subdirectory(utils)
add_subdirectory(net)

add_executable(HServer ${DIR_SRCS})

# 添加链接库
target_link_libraries(HServer ${CMAKE_THREAD_LIBS_INIT} utils_lib net_lib)