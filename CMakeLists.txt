cmake_minimum_required(VERSION 3.24)

project(noitasettings
    HOMEPAGE_URL "https://github.com/dextercd/NoitaSettings"
    LANGUAGES C CXX
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
