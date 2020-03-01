CMAKE_MINIMUM_REQUIRED(VERSION 3.4.0)
INCLUDE(GNUInstallDirs)
PROJECT(hiredis_vip)

IF (WIN32 OR MINGW)
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

SET(hiredis_vip_SOURCES
    adlist.c
    command.c
    crc16.c
    hiarray.c
    hircluster.c
    hiutil.c)
IF(WIN32 OR MINGW)
    SET(hiredis_vip_SOURCES
        ${hiredis_vip_SOURCES}
        hiredis_vip.def)
ENDIF()

ADD_LIBRARY(hiredis_vip SHARED
    ${hiredis_vip_SOURCES})

SET_TARGET_PROPERTIES(hiredis_vip
    PROPERTIES
    VERSION "${HIREDIS_VIP_SONAME}")

FIND_PACKAGE(hiredis REQUIRED)

TARGET_INCLUDE_DIRECTORIES(hiredis_vip PUBLIC ${hiredis_INCLUDE_DIRS})

IF(WIN32 OR MINGW)
    TARGET_LINK_LIBRARIES(hiredis_vip PRIVATE ws2_32 hiredis::hiredis)
ENDIF()

CONFIGURE_FILE(hiredis_vip.pc.in hiredis_vip.pc @ONLY)

INSTALL(TARGETS hiredis_vip
        EXPORT hiredis_vip-targets)

INSTALL(FILES hircluster.h adlist.h hiarray.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hiredis_vip)
    
INSTALL(DIRECTORY adapters
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hiredis_vip)
    
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/hiredis_vip.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

export(EXPORT hiredis_vip-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/hiredis_vip-targets.cmake
       NAMESPACE hiredis_vip::)

SET(CMAKE_CONF_INSTALL_DIR share/hiredis_vip)
SET(INCLUDE_INSTALL_DIR include)
include(CMakePackageConfigHelpers)
configure_package_config_file(hiredis_vip-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/hiredis_vip-config.cmake
                              INSTALL_DESTINATION ${CMAKE_CONF_INSTALL_DIR}
                              PATH_VARS INCLUDE_INSTALL_DIR)

INSTALL(EXPORT hiredis_vip-targets
        FILE hiredis_vip-targets.cmake
        NAMESPACE hiredis_vip::
        DESTINATION ${CMAKE_CONF_INSTALL_DIR})

INSTALL(FILES hiredis_vip-config.cmake
        DESTINATION ${CMAKE_CONF_INSTALL_DIR})


