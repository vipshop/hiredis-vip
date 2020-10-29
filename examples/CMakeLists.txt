cmake_minimum_required(VERSION 3.14)

project(examples CXX)

include(ExternalProject)

# Handle libevent and hiredis
find_library(EVENT_LIBRARY event HINTS /usr/lib/x86_64-linux-gnu)
find_package(hiredis REQUIRED)
find_package(hiredis_vip REQUIRED)

include_directories("${hiredis_INCLUDE_DIRS}")
include_directories("${hiredis_vip_INCLUDE_DIRS}")

# Executable: IPv4
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${hiredis_LIBRARIES} ${hiredis_vip_LIBRARIES})

# Executable: IPv6
add_executable(${PROJECT_NAME}_ipv6 main_ipv6.cpp)
target_link_libraries(${PROJECT_NAME}_ipv6 ${hiredis_LIBRARIES} ${hiredis_vip_LIBRARIES})

# Executable: async
add_executable(${PROJECT_NAME}_async main_async.cpp)
target_link_libraries(${PROJECT_NAME}_async ${hiredis_LIBRARIES} ${hiredis_vip_LIBRARIES} ${EVENT_LIBRARY})

# Executable: tls
add_executable(${PROJECT_NAME}_tls main_async.cpp)
target_link_libraries(${PROJECT_NAME}_tls ${hiredis_LIBRARIES} ${hiredis_vip_LIBRARIES} ${EVENT_LIBRARY})
