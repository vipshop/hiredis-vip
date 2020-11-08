cmake_minimum_required(VERSION 3.14)
project(examples LANGUAGES CXX)

# Handle libevent and hiredis
find_library(EVENT_LIBRARY event HINTS /usr/lib/x86_64-linux-gnu)
find_package(hiredis REQUIRED)
find_package(hiredis_cluster REQUIRED)

include_directories("${hiredis_INCLUDE_DIRS}")
include_directories("${hiredis_cluster_INCLUDE_DIRS}")

# Executable: IPv4
add_executable(example_ipv4 main.cpp)
target_link_libraries(example_ipv4
  ${hiredis_LIBRARIES}
  ${hiredis_cluster_LIBRARIES})

# Executable: async
add_executable(example_async main_async.cpp)
target_link_libraries(example_async
  ${hiredis_LIBRARIES}
  ${hiredis_cluster_LIBRARIES}
  ${EVENT_LIBRARY})
