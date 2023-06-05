cmake_minimum_required(VERSION 3.24)

project(NoitaData
    HOMEPAGE_URL "https://github.com/dextercd/NoitaSettings"
    LANGUAGES C CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(fastlz
    GIT_REPOSITORY https://github.com/ariya/FastLZ.git
    GIT_TAG 344eb4025f9ae866ebf7a2ec48850f7113a97a42
)

FetchContent_MakeAvailable(fastlz)

add_library(fastlz
    ${fastlz_SOURCE_DIR}/fastlz.c
    ${fastlz_SOURCE_DIR}/fastlz.h
)

target_include_directories(fastlz PUBLIC ${fastlz_SOURCE_DIR})

# Common

add_library(noita-settings-common
    utils.cpp
)

target_link_libraries(noita-settings-common
    PUBLIC
        fastlz
)

# Programs

add_executable(noita-settings-dump settings_main.cpp)
target_link_libraries(noita-settings-dump PUBLIC noita-settings-common)

add_executable(noita-decompress decompress_main.cpp)
target_link_libraries(noita-decompress PUBLIC noita-settings-common)

find_package(SFML REQUIRED system window graphics)

add_executable(noita-png-petri png_petri_main.cpp)
target_link_libraries(noita-png-petri
    PUBLIC
        noita-settings-common
        sfml-system
        sfml-graphics
        sfml-window
)

install(TARGETS noita-settings-dump noita-decompress noita-png-petri
    DESTINATION "NoitaData"
    COMPONENT NoitaData
)

# Packaging
set(CPACK_VERBATIM_VARIABLES TRUE)
set(CPACK_GENERATOR ZIP)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
include(CPack)
