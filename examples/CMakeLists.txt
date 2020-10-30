cmake_minimum_required(VERSION 3.14)

project(hiredis_vip_externalproject C)

include(ExternalProject)

ExternalProject_Add(hiredis
  PREFIX hiredis
  GIT_REPOSITORY  https://github.com/redis/hiredis
  GIT_TAG         v1.0.0
  CMAKE_CACHE_ARGS
    "-DENABLE_SSL:BOOL=ON"
    "-DCMAKE_BUILD_TYPE:STRING=Debug"
    "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/hiredis"
)

ExternalProject_Add(hiredis_vip
  PREFIX hiredis_vip
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."
  CMAKE_CACHE_ARGS
    "-DCMAKE_BUILD_TYPE:STRING=Debug"
    "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/hiredis_vip"
    "-Dhiredis_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/hiredis/share/hiredis"
  DEPENDS hiredis
)

ExternalProject_Add(examples
  PREFIX examples
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src"
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    "-DCMAKE_BUILD_TYPE:STRING=Debug"
    "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/examples"
    "-Dhiredis_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/hiredis/share/hiredis"
    "-Dhiredis_vip_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/hiredis_vip/share/hiredis_vip"
  DEPENDS hiredis_vip
)
