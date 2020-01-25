CMAKE_MINIMUM_REQUIRED(VERSION 3.4.0)
INCLUDE(GNUInstallDirs)
PROJECT(hiredis_vip)

OPTION(ENABLE_SSL "Build hiredis_ssl for SSL support" OFF)
OPTION(DISABLE_TESTS "If tests should be compiled or not" OFF)

IF (WIN32 OR MINGW)
    SET(BUILD_SHARED_LIBS OFF)
    ADD_COMPILE_DEFINITIONS(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)
ENDIF()

IF (MSVC)
    ADD_COMPILE_OPTIONS("/wd 4267" "/wd 4244")
ENDIF()

MACRO(getVersionBit name)
  SET(VERSION_REGEX "^#define ${name} (.+)$")
  FILE(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/hircluster.h"
    VERSION_BIT REGEX ${VERSION_REGEX})
  STRING(REGEX REPLACE ${VERSION_REGEX} "\\1" ${name} "${VERSION_BIT}")
ENDMACRO(getVersionBit)

getVersionBit(HIREDIS_VIP_MAJOR)
getVersionBit(HIREDIS_VIP_MINOR)
getVersionBit(HIREDIS_VIP_PATCH)
getVersionBit(HIREDIS_VIP_SONAME)
SET(VERSION "${HIREDIS_VIP_MAJOR}.${HIREDIS_VIP_MINOR}.${HIREDIS_VIP_PATCH}")
MESSAGE("Detected version: ${VERSION}")

PROJECT(hiredis_vip VERSION "${VERSION}")

SET(ENABLE_EXAMPLES OFF CACHE BOOL "Enable building hiredis examples")

ADD_LIBRARY(hiredis_vip
    async.c
    dict.c
    hiredis.c
    net.c
    read.c
    sds.c
    sockcompat.c
    adlist.c
    command.c
    crc16.c
    hiarray.c
    hircluster.c
    hiutil.c)

SET_TARGET_PROPERTIES(hiredis_vip
    PROPERTIES
    VERSION "${HIREDIS_VIP_SONAME}")
IF(WIN32 OR MINGW)
    TARGET_LINK_LIBRARIES(hiredis_vip PRIVATE ws2_32)
ENDIF()
TARGET_INCLUDE_DIRECTORIES(hiredis_vip PUBLIC .)

CONFIGURE_FILE(hiredis_vip.pc.in hiredis_vip.pc @ONLY)

INSTALL(TARGETS hiredis_vip
    DESTINATION "${CMAKE_INSTALL_LIBDIR}")

INSTALL(FILES hiredis.h read.h sds.h async.h hircluster.h dict.h adlist.h hiarray.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hiredis_vip)
    
INSTALL(DIRECTORY adapters
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hiredis_vip)
    
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/hiredis_vip.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

IF(ENABLE_SSL)
    IF (NOT OPENSSL_ROOT_DIR)
        IF (APPLE)
            SET(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
        ENDIF()
    ENDIF()
    FIND_PACKAGE(OpenSSL REQUIRED)
    ADD_LIBRARY(hiredis_ssl
        ssl.c)
    TARGET_INCLUDE_DIRECTORIES(hiredis_ssl PRIVATE "${OPENSSL_INCLUDE_DIR}")
    TARGET_LINK_LIBRARIES(hiredis_ssl PRIVATE ${OPENSSL_LIBRARIES})
    CONFIGURE_FILE(hiredis_ssl.pc.in hiredis_ssl.pc @ONLY)

    INSTALL(TARGETS hiredis_ssl
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

    INSTALL(FILES hiredis_ssl.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hiredis_vip)
    
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/hiredis_ssl.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
ENDIF()

IF(NOT (DISABLE_TESTS OR (WIN32 OR MINGW)))
    ENABLE_TESTING()
    ADD_EXECUTABLE(hiredis-test test.c)
    TARGET_LINK_LIBRARIES(hiredis-test hiredis_vip)
    ADD_TEST(NAME hiredis-test
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test.sh)
ENDIF()

# Add examples
IF(ENABLE_EXAMPLES)
  ADD_SUBDIRECTORY(examples)
ENDIF(ENABLE_EXAMPLES)
