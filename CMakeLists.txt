cmake_minimum_required (VERSION 3.19)

project(kaos-public)

macro(kaos_public_option OPTION_NAME OPTION_TEXT OPTION_DEFAULT)
    option(${OPTION_NAME} ${OPTION_TEXT} ${OPTION_DEFAULT})
    if(DEFINED ENV{${OPTION_NAME}})
        # Allow overriding the option through an environment variable
        set(${OPTION_NAME} $ENV{${OPTION_NAME}})
    endif()
    if(${OPTION_NAME})
        add_definitions(-D${OPTION_NAME})
    endif()
    message(STATUS "${OPTION_NAME}: ${${OPTION_NAME}}")
endmacro()

kaos_public_option(KAOS_PUBLIC_PRECOMPILED_HEADERS "Enable precompiled headers." ON)

include(FetchContent)
include(GNUInstallDirs)

# ------ brotli ------
set(BROTLI_DISABLE_TESTS ON)
set(BROTLI_BUNDLED_MODE ON)

FetchContent_Declare(
	brotli
	GIT_REPOSITORY https://github.com/google/brotli
	GIT_TAG v1.0.9)
FetchContent_MakeAvailable(brotli)	

set(BROTLI_LIBRARIES_STATIC brotlidec-static brotlienc-static)

# -------------------

include_directories(extra)

add_subdirectory(extra)
add_subdirectory(lib)
add_subdirectory(include)
add_subdirectory(util)


