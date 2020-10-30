cmake_minimum_required(VERSION 3.14)

option(DOWNLOAD_HIREDIS "Download the dependency hiredis from GitHub" ON)
option(DISABLE_TESTS    "Disable compilation of test" OFF)

MACRO(getVersionBit name)
  SET(VERSION_REGEX "^#define ${name} (.+)$")
  FILE(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/hircluster.h"
    VERSION_BIT REGEX ${VERSION_REGEX})
  STRING(REGEX REPLACE ${VERSION_REGEX} "\\1" ${name} "${VERSION_BIT}")
ENDMACRO(getVersionBit)

# Get version information from src-code
getVersionBit(HIREDIS_VIP_MAJOR)
getVersionBit(HIREDIS_VIP_MINOR)
getVersionBit(HIREDIS_VIP_PATCH)
getVersionBit(HIREDIS_VIP_SONAME)
SET(VERSION "${HIREDIS_VIP_MAJOR}.${HIREDIS_VIP_MINOR}.${HIREDIS_VIP_PATCH}")
MESSAGE("Detected version: ${VERSION}")

project(hiredis_vip
  VERSION "${VERSION}"
  LANGUAGES C)

SET(hiredis_vip_SOURCES
    adlist.c
    command.c
    crc16.c
    hiarray.c
    hircluster.c
    hiutil.c)

IF(WIN32 OR MINGW)
    ADD_COMPILE_DEFINITIONS(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)
    SET(hiredis_vip_SOURCES
        ${hiredis_vip_SOURCES}
        hiredis_vip.def)
ENDIF()

add_library(hiredis_vip
  SHARED
  ${hiredis_vip_SOURCES})

if(MSVC)
  # MS Visual: Suppress warnings
  target_compile_options(hiredis_vip PRIVATE "/wd 4267" "/wd 4244")
else()
  target_compile_options(hiredis_vip PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

SET_TARGET_PROPERTIES(hiredis_vip
    PROPERTIES
    VERSION "${HIREDIS_VIP_SONAME}")

if(DOWNLOAD_HIREDIS)
  message("Downloading dependency 'hiredis'..")

  include(FetchContent)
  FetchContent_Declare(hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis
    GIT_TAG        v1.0.0
    SOURCE_DIR     "${CMAKE_CURRENT_BINARY_DIR}/_deps/hiredis"
  )

  # Disable tests in hiredis
  set(DISABLE_TESTS_OLD ${DISABLE_TESTS})
  set(DISABLE_TESTS ON CACHE INTERNAL "")
  FetchContent_MakeAvailable(hiredis)
  set(DISABLE_TESTS ${DISABLE_TESTS_OLD} CACHE INTERNAL "")

  # Create an empty *Config.cmake for find_package
  # See: https://github.com/abandonware-pjz37/cmake-find-package-include/blob/master/hooks/fetch.cmake
  set(stub_dir "${CMAKE_CURRENT_BINARY_DIR}/generated/pkg")
  file(WRITE "${stub_dir}/hiredisConfig.cmake" "")
  set(hiredis_DIR ${stub_dir})

  # Set variables normally got from hiredisConfig.cmake
  SET(hiredis_LIBRARIES hiredis::hiredis)
  SET(hiredis_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/_deps")
else()
  message("Expecting to find dependency 'hiredis' in path..")
endif()

find_package(hiredis REQUIRED)

target_include_directories(hiredis_vip PUBLIC
  $<BUILD_INTERFACE:${hiredis_INCLUDE_DIRS}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

IF(WIN32 OR MINGW)
    TARGET_LINK_LIBRARIES(hiredis_vip PRIVATE ws2_32 hiredis::hiredis)
ENDIF()

if (NOT DISABLE_TESTS)
  include(CTest)

  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()
endif()

# Get CMAKE_INSTALL_ variables
include(GNUInstallDirs)

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

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/hiredis_vip-config.cmake
        DESTINATION ${CMAKE_CONF_INSTALL_DIR})
