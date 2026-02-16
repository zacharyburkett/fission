cmake_minimum_required(VERSION 3.20)
project(fission VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)

option(
    FISSION_NUKLEAR_AUTO_FETCH
    "Automatically fetch Nuklear when building fission"
    ON
)

set(
    FISSION_NUKLEAR_INCLUDE_DIR
    ""
    CACHE PATH
    "Directory containing nuklear.h"
)

find_path(
    FISSION_NUKLEAR_INCLUDE_DIR
    nuklear.h
    HINTS ${FISSION_NUKLEAR_INCLUDE_DIR}
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        /opt/homebrew/include
        /usr/local/include
)

if(NOT FISSION_NUKLEAR_INCLUDE_DIR AND FISSION_NUKLEAR_AUTO_FETCH)
    include(FetchContent)
    message(STATUS "FISSION: fetching Nuklear sources")
    FetchContent_Declare(
        fission_nuklear
        GIT_REPOSITORY https://github.com/vurtun/nuklear.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(fission_nuklear)
    if(EXISTS "${fission_nuklear_SOURCE_DIR}/nuklear.h")
        set(FISSION_NUKLEAR_INCLUDE_DIR "${fission_nuklear_SOURCE_DIR}")
    endif()
endif()

if(NOT FISSION_NUKLEAR_INCLUDE_DIR)
    message(FATAL_ERROR
        "Fission requires nuklear.h. "
        "Set FISSION_NUKLEAR_INCLUDE_DIR or enable FISSION_NUKLEAR_AUTO_FETCH."
    )
endif()

add_library(fission src/nuklear_ui.c)
add_library(fission::fission ALIAS fission)

target_compile_features(fission PUBLIC c_std_11)
target_include_directories(
    fission
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${FISSION_NUKLEAR_INCLUDE_DIR}
)

if(MSVC)
    target_compile_options(fission PRIVATE /W4 /WX)
else()
    target_compile_options(fission PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

install(
    TARGETS fission
    EXPORT fissionTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
    EXPORT fissionTargets
    FILE fissionTargets.cmake
    NAMESPACE fission::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fission
)
