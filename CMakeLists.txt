cmake_minimum_required(VERSION 3.15)
project(xlab)

# ──────────────────────────────────────
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(PD_BUILD_STATIC_OBJECTS OFF)
set(POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(IAMCHARLES "Set to ON if you are Charles K. Neimog, this use local packages" OFF)

# ╭──────────────────────────────────────╮
# │                FLAGS                 │
# ╰──────────────────────────────────────╯
if(LINUX OR MINGW)
    add_link_options(-static-libgcc -static-libstdc++)
endif()

# ╭──────────────────────────────────────╮
# │               pd.cmake               │
# ╰──────────────────────────────────────╯
set(PDCMAKE_FILE ${CMAKE_BINARY_DIR}/pd.cmake)
set(PDCMAKE_VERSION "v0.2.15")
if(NOT EXISTS ${PDCMAKE_FILE})
    message(STATUS "Downloading pd.cmake")
    file(DOWNLOAD https://raw.githubusercontent.com/pure-data/pd.cmake/refs/heads/main/pd.cmake ${PDCMAKE_FILE}
         STATUS DOWNLOAD_STATUS)

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

file(
    ARCHIVE_EXTRACT
    INPUT
    ${CMAKE_BINARY_DIR}/fftw-3.3.10.tar.gz
    DESTINATION
    ${CMAKE_BINARY_DIR}/)

add_subdirectory(${CMAKE_BINARY_DIR}/fftw-3.3.10 ${CMAKE_BINARY_DIR}/fftw-3.3.10-build EXCLUDE_FROM_ALL)
get_property(
    all_targets
    DIRECTORY ${CMAKE_BINARY_DIR}/fftw-3.3.10
    PROPERTY BUILDSYSTEM_TARGETS)

set_target_properties(fftw3f PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(fftw3f PROPERTIES POSITION_INDEPENDENT_CODE ON)

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

set(PYTHON OFF)
cpmaddpackage("gh:charlesneimog/loris#master")
set_target_properties(
    loris
    PROPERTIES POSITION_INDEPENDENT_CODE ON
               CXX_STANDARD_REQUIRED ON
               CXX_EXTENSIONS OFF)
target_include_directories(loris PRIVATE ${CMAKE_BINARY_DIR}/fftw-3.3.10/api)

# freeze
pd_add_external(
    lfreeze~
    "${CMAKE_CURRENT_SOURCE_DIR}/src/spectral/lfreeze~.cpp"
    LINK_LIBRARIES
    loris)

# main object
pd_add_external(xlab "${CMAKE_CURRENT_SOURCE_DIR}/src/xlab.cpp")

pd_add_external(
    xfreeze~
    "${CMAKE_CURRENT_SOURCE_DIR}/src/fft/xfreeze~.cpp"
    LINK_LIBRARIES
    fftw3f)

pd_add_external(
    gfreeze~
    "${CMAKE_CURRENT_SOURCE_DIR}/src/fft/gfreeze~.cpp"
    LINK_LIBRARIES
    fftw3f)

target_include_directories(xfreeze_tilde PRIVATE ${CMAKE_BINARY_DIR}/fftw-3.3.10/api)
target_include_directories(gfreeze_tilde PRIVATE ${CMAKE_BINARY_DIR}/fftw-3.3.10/api)
target_include_directories(manipulations PRIVATE ${CMAKE_BINARY_DIR}/fftw-3.3.10/api)

include_directories(${CMAKE_BINARY_DIR}/)

pd_add_external(xclick~ "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/xclick~.cpp")

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

    # add_subdirectory(../pd-onnx ${CMAKE_BINARY_DIR}/pd-onnx)
    add_subdirectory(../onsetsds_tilde ${CMAKE_BINARY_DIR}/onsetsds_tilde)
    add_subdirectory(../dragonfly_tilde ${CMAKE_BINARY_DIR}/dragonfly_tilde)

    add_subdirectory(../pd-saf ${CMAKE_BINARY_DIR}/pd-saf)
    # add_subdirectory(../pd-soundtouch ${CMAKE_BINARY_DIR}/pd-soundtouch)
else()
    if(NOT EMSCRIPTEN)
        cpmaddpackage("gh:charlesneimog/pd-ambi#main")
        cpmaddpackage("gh:charlesneimog/py4pd#master")
        cpmaddpackage("gh:EL-LOCUS-SOLUS/pd-lua#main")
    endif()

    cpmaddpackage("gh:charlesneimog/pv_tilde#main")
    cpmaddpackage("gh:charlesneimog/mar_tilde#main")
    cpmaddpackage("gh:charlesneimog/pd-stk#main")

    # lua
    cpmaddpackage("gh:charlesneimog/pd-upic#main")
    cpmaddpackage("gh:charlesneimog/pd-orchidea#main")
    cpmaddpackage("gh:EL-LOCUS-SOLUS/pd-bhack#main")

    # single objects cpmaddpackage("gh:charlesneimog/pd-onnx#main")
    cpmaddpackage("gh:charlesneimog/onsetsds_tilde#main")
    cpmaddpackage("gh:charlesneimog/pd-dragonfly_tilde#main")

    # saf
    cpmaddpackage("gh:EL-LOCUS-SOLUS/pd-saf#main")
endif()

target_link_libraries(
    xlab
    PRIVATE utilities
            manipulations
            arrays
            statistics
            lua)

# ╭──────────────────────────────────────╮
# │ Lua, Python Objects + Audio and Data │
# ╰──────────────────────────────────────╯
file(GLOB LUA_OBJECTS "${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.pd_lua")
pd_add_datafile(xlab "${LUA_OBJECTS}")

file(GLOB PYTHON_OBJECTS "${CMAKE_CURRENT_SOURCE_DIR}/src/python/*.pd_py")
pd_add_datafile(xlab "${PYTHON_OBJECTS}")

file(GLOB AUDIO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/audios/*.wav")
pd_add_datafile(xlab "${AUDIO_FILES}")

file(GLOB ABS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/abs/*.pd")
pd_add_datafile(xlab "${ABS_FILES}")

# ╭──────────────────────────────────────╮
# │             xlab library             │
# ╰──────────────────────────────────────╯
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_target(
    edit_files
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/resources/edit_patches.py ${CMAKE_BINARY_DIR}/xlab
    COMMENT "Editing generated files with Python")

add_dependencies(
    edit_files # target
    # fft
    xfreeze_tilde
    gfreeze_tilde
    # lua
    upic
    orchidea
    bhack
    # single
    pv_tilde
    mar_tilde
    onsetsds_tilde
    dragonfly_tilde
    saf
    saf.encoder_tilde
    saf.decoder_tilde
    saf.binaural_tilde
    saf.roomsim_tilde
    saf.panner_tilde)

if(NOT EMSCRIPTEN)
    add_dependencies(edit_files py4pd)
endif()

add_dependencies(xlab edit_files)
