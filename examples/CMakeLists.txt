cmake_minimum_required(VERSION 3.0)
project(hiredis-vip-example C)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/../
)

add_executable(example_cluster example_cluster.c)
target_link_libraries(example_cluster hiredis-vip_static)