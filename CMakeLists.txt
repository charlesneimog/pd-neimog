cmake_minimum_required(VERSION 3.15)
project(xlab)

# ──────────────────────────────────────
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(PD_BUILD_STATIC_OBJECTS OFF)
set(POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


option(IAMCHARLES "Set to ON if you are me, this use local packages" OFF)

# ╭──────────────────────────────────────╮
# │                FLAGS                 │
# ╰──────────────────────────────────────╯
if(LINUX OR MINGW)
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static-libgfortran")

    set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static-libgfortran")
endif()

# ╭──────────────────────────────────────╮
# │               pd.cmake               │
# ╰──────────────────────────────────────╯
set(PDCMAKE_FILE ${CMAKE_BINARY_DIR}/pd.cmake)
set(PDCMAKE_VERSION "v0.2.11")
if(NOT EXISTS ${PDCMAKE_FILE})
    message(STATUS "Downloading pd.cmake")
    file(DOWNLOAD
         https://raw.githubusercontent.com/pure-data/pd.cmake/refs/heads/main/pd.cmake
         ${PDCMAKE_FILE} STATUS DOWNLOAD_STATUS)

endif()
include(${PDCMAKE_FILE})

include_directories("${PD_SOURCES_PATH}")

# ╭──────────────────────────────────────╮
# │                 CPM                  │
# ╰──────────────────────────────────────╯
set(CPM_FILE ${CMAKE_BINARY_DIR}/CPM.cmake)
set(CPM_VERSION "0.42.1")
if(NOT EXISTS "${CPM_FILE}")
    file(DOWNLOAD "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake" ${CPM_FILE})
endif()
include(${CPM_FILE})

# ╭──────────────────────────────────────╮
# │                FFTW3                 │
# ╰──────────────────────────────────────╯
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_FLOAT
    ON
    CACHE BOOL "Build single-precision (fftw3f)" FORCE)
set(ENABLE_LONG_DOUBLE
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_QUAD_PRECISION
    OFF
    CACHE BOOL "" FORCE)
set(DISABLE_FORTRAN
    ON
    CACHE BOOL "" FORCE)

set(FFTW3_FILE ${CMAKE_BINARY_DIR}/fftw-3.3.10.tar.gz)
if(NOT EXISTS ${FFTW3_FILE})
    message(STATUS "Downloading FFTW3")
    file(DOWNLOAD https://www.fftw.org/fftw-3.3.10.tar.gz ${FFTW3_FILE} STATUS DOWNLOAD_STATUS)
endif()

file(ARCHIVE_EXTRACT INPUT ${CMAKE_BINARY_DIR}/fftw-3.3.10.tar.gz DESTINATION ${CMAKE_BINARY_DIR}/)
add_subdirectory(${CMAKE_BINARY_DIR}/fftw-3.3.10 EXCLUDE_FROM_ALL)
get_property(
    all_targets
    DIRECTORY ${CMAKE_BINARY_DIR}/fftw-3.3.10
    PROPERTY BUILDSYSTEM_TARGETS)

set_target_properties(fftw3f PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(fftw3f PROPERTIES POSITION_INDEPENDENT_CODE ON)
include_directories(${CMAKE_BINARY_DIR}/fftw-3.3.10/api)

# ╭──────────────────────────────────────╮
# │               Objects                │
# ╰──────────────────────────────────────╯
file(GLOB statistics_src "${CMAKE_CURRENT_SOURCE_DIR}/src/statistics/*.cpp")
add_library(statistics STATIC "${statistics_src}")
set_target_properties(statistics PROPERTIES POSITION_INDEPENDENT_CODE ON)

# array
file(GLOB arrays_src "${CMAKE_CURRENT_SOURCE_DIR}/src/arrays/*.cpp")
add_library(arrays STATIC "${arrays_src}")
set_target_properties(arrays PROPERTIES POSITION_INDEPENDENT_CODE ON)

# manipulations
file(GLOB manipulations "${CMAKE_CURRENT_SOURCE_DIR}/src/manipulations/*.cpp")
add_library(manipulations STATIC "${manipulations}")
set_target_properties(manipulations PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(manipulations PUBLIC fftw3f)

# mir
file(GLOB mir_source "${CMAKE_CURRENT_SOURCE_DIR}/src/mir/*.cpp")
add_library(mir STATIC "${mir_source}")
set_target_properties(mir PROPERTIES POSITION_INDEPENDENT_CODE ON)

# utilities
file(GLOB utilities_src "${CMAKE_CURRENT_SOURCE_DIR}/src/utilities/*.cpp")
add_library(utilities STATIC "${utilities_src}")
set_target_properties(utilities PROPERTIES POSITION_INDEPENDENT_CODE ON)

# main object
pd_add_external(xlab "${CMAKE_CURRENT_SOURCE_DIR}/src/xlab.cpp")

# ╭──────────────────────────────────────╮
# │         Externals Libraries          │
# ╰──────────────────────────────────────╯

if(IAMCHARLES)
    add_subdirectory(../pd-ambi ${CMAKE_BINARY_DIR}/pd-ambi)
    add_subdirectory(../pv~ ${CMAKE_BINARY_DIR}/pv~)
    add_subdirectory(../mar_tilde ${CMAKE_BINARY_DIR}/mar_tilde)
    add_subdirectory(../pd-stk ${CMAKE_BINARY_DIR}/pd-stk)

    add_subdirectory(../pd-lua ${CMAKE_BINARY_DIR}/pd-lua)
    add_subdirectory(../py4pd ${CMAKE_BINARY_DIR}/py4pd)
    add_subdirectory(../pd-upic ${CMAKE_BINARY_DIR}/pd-upic)
    add_subdirectory(../pd-orchidea ${CMAKE_BINARY_DIR}/pd-orchidea)
    add_subdirectory(../pd-bhack ${CMAKE_BINARY_DIR}/pd-bhack)

    add_subdirectory(../pd-onnx ${CMAKE_BINARY_DIR}/pd-onnx)
    add_subdirectory(../onsetsds_tilde ${CMAKE_BINARY_DIR}/onsetsds_tilde)
    add_subdirectory(../dragonfly_tilde ${CMAKE_BINARY_DIR}/dragonfly_tilde)

    add_subdirectory(../pd-saf ${CMAKE_BINARY_DIR}/pd-saf)
else()
    cpmaddpackage("gh:charlesneimog/pd-ambi#main")
    cpmaddpackage("gh:charlesneimog/pv_tilde#main")
    cpmaddpackage("gh:charlesneimog/mar_tilde#main")
    cpmaddpackage("gh:charlesneimog/pd-stk#main")
    cpmaddpackage("gh:charlesneimog/py4pd#master")

    # lua
    cpmaddpackage("gh:EL-LOCUS-SOLUS/pd-lua#54e941ff901473986a5e4ce1197c0dbc0d96e3ed")
    cpmaddpackage("gh:charlesneimog/pd-upic#main")
    cpmaddpackage("gh:charlesneimog/pd-orchidea#main")
    cpmaddpackage("gh:EL-LOCUS-SOLUS/pd-bhack#main")

    # single objects
    cpmaddpackage("gh:charlesneimog/pd-onnx#main")
    cpmaddpackage("gh:charlesneimog/onsetsds_tilde#v0.1.0")

    # saf
    cpmaddpackage("gh:EL-LOCUS-SOLUS/pd-saf#main")
endif()

target_link_libraries(xlab PRIVATE utilities manipulations arrays statistics pdlua)

# ╭──────────────────────────────────────╮
# │             xlab library             │
# ╰──────────────────────────────────────╯
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_target(
    edit_files
    # COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/resources/edit_patches.py ${CMAKE_BINARY_DIR}/xlab
    COMMENT "Editing generated files with Python")

add_dependencies(
    edit_files # target

    # lua
    upic
    orchidea
    bhack

    # single
    pdonnx
    pv_tilde
    mar_tilde
    onsetsds_tilde
    py4pd

    # saf
    saf.encoder_tilde
    saf.decoder_tilde
    saf.binaural_tilde
    saf.roomsim_tilde
    saf.panner_tilde
)

add_dependencies(xlab edit_files)
