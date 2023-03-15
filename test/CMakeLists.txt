# vim: ts=4:sw=4:expandtab
CMAKE_MINIMUM_REQUIRED(VERSION 3.20...3.23)

## Check user set options.
IF(NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "CMAKE_BUILD_TYPE was not set by user; setting build type to Debug")
    SET(CMAKE_BUILD_TYPE "Debug")
ELSE()
    # List of valid build types
    SET(VALID_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
    LIST(FIND VALID_BUILD_TYPES ${CMAKE_BUILD_TYPE} IS_VALID_BUILD_TYPE)
    IF(IS_VALID_BUILD_TYPE EQUAL -1)
        MESSAGE(FATAL_ERROR "CMAKE_BUILD_TYPE was '${CMAKE_BUILD_TYPE}' but can only be set to one of ${VALID_BUILD_TYPES}")
    ENDIF()
ENDIF()

# Obtain version
INCLUDE(../version.all)

# Initialise project information.
PROJECT(peanut-benchmark
        LANGUAGES C
	VERSION
	${PEANUTGB_VERSION_MAJOR}.${PEANUTGB_VERSION_MINOR}.${PEANUTGB_VERSION_PATCH}
        DESCRIPTION "Peanut-GB ROM Test Application"
        HOMEPAGE_URL "https://github.com/deltabeard/peanut-gb")

# Add dependencies to project.
IF(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug" AND APPLE)
    SET(EXE_TARGET_TYPE MACOSX_BUNDLE)
    MESSAGE(VERBOSE "Setting EXE type to MACOSX Bundle")
ELSEIF(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug" AND MSVC)
    SET(EXE_TARGET_TYPE WIN32)
    MESSAGE(VERBOSE "Setting EXE type to WIN32")
ENDIF()

ADD_EXECUTABLE(test_external_rom ${EXE_TARGET_TYPE} test_external_rom.c ../peanut_gb.h)
TARGET_INCLUDE_DIRECTORIES(test_external_rom PRIVATE ../)

MESSAGE(STATUS "  CC:      ${CMAKE_C_COMPILER} '${CMAKE_C_COMPILER_ID}' on '${CMAKE_SYSTEM_NAME}'")
MESSAGE(STATUS "  CFLAGS:  ${CMAKE_C_FLAGS}")
MESSAGE(STATUS "  LDFLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
