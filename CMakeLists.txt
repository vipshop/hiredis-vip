cmake_minimum_required(VERSION 3.0)
project(hiredis-vip C)

set(
    INC_LIST
    ${CMAKE_CURRENT_LIST_DIR}/adlist.h
    ${CMAKE_CURRENT_LIST_DIR}/async.h
    ${CMAKE_CURRENT_LIST_DIR}/command.h
    ${CMAKE_CURRENT_LIST_DIR}/dict.h
    ${CMAKE_CURRENT_LIST_DIR}/fmacros.h
    ${CMAKE_CURRENT_LIST_DIR}/hiarray.h
    ${CMAKE_CURRENT_LIST_DIR}/hircluster.h
    ${CMAKE_CURRENT_LIST_DIR}/hiredis.h
    ${CMAKE_CURRENT_LIST_DIR}/hiutil.h
    ${CMAKE_CURRENT_LIST_DIR}/net.h
    ${CMAKE_CURRENT_LIST_DIR}/read.h
    ${CMAKE_CURRENT_LIST_DIR}/sds.h
    ${CMAKE_CURRENT_LIST_DIR}/win32.h
)
set(
    SRC_LIST
    ${CMAKE_CURRENT_LIST_DIR}/adlist.c
    ${CMAKE_CURRENT_LIST_DIR}/async.c
    ${CMAKE_CURRENT_LIST_DIR}/command.c
    ${CMAKE_CURRENT_LIST_DIR}/crc16.c
    ${CMAKE_CURRENT_LIST_DIR}/dict.c
    ${CMAKE_CURRENT_LIST_DIR}/hiarray.c
    ${CMAKE_CURRENT_LIST_DIR}/hircluster.c
    ${CMAKE_CURRENT_LIST_DIR}/hiredis.c
    ${CMAKE_CURRENT_LIST_DIR}/hiutil.c
    ${CMAKE_CURRENT_LIST_DIR}/net.c
    ${CMAKE_CURRENT_LIST_DIR}/read.c
    ${CMAKE_CURRENT_LIST_DIR}/sds.c
)

set(LIB_NAME ${PROJECT_NAME}_static)
add_library(
    ${LIB_NAME}
    ${INC_LIST}
    ${SRC_LIST}
)

install_files(
    /include/hiredis-vip
    FILES
    ${INC_LIST}
)

install_targets(/lib ${LIB_NAME})
