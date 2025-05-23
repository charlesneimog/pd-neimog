cmake_minimum_required(VERSION 3.15)
include(${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.cmake/pd.cmake) # pd.build
project(neimog)

# ╭──────────────────────────────────────╮
# │                FFTW3                 │
# ╰──────────────────────────────────────╯
cmake_policy(SET CMP0135 NEW)
option(BUILD_SHARED_LIBS OFF)
option(BUILD_TESTS OFF)
set(FFTW3_FILE ${CMAKE_BINARY_DIR}/fftw-3.3.10.tar.gz)
if(NOT EXISTS ${FFTW3_FILE})
  message(STATUS "Downloading FFTW3")
  file(
    DOWNLOAD https://www.fftw.org/fftw-3.3.10.tar.gz ${FFTW3_FILE}
    SHOW_PROGRESS
    STATUS DOWNLOAD_STATUS)
endif()

file(ARCHIVE_EXTRACT INPUT ${CMAKE_BINARY_DIR}/fftw-3.3.10.tar.gz DESTINATION
     ${CMAKE_BINARY_DIR}/)

add_subdirectory(${CMAKE_BINARY_DIR}/fftw-3.3.10 EXCLUDE_FROM_ALL)
set_target_properties(fftw3 PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                       ${CMAKE_BINARY_DIR})
set_target_properties(fftw3 PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ╭──────────────────────────────────────╮
# │               Objects                │
# ╰──────────────────────────────────────╯
# divergence
file(GLOB statistics_src ${CMAKE_CURRENT_SOURCE_DIR}/Sources/statistics/*.cpp)
add_library(statistics STATIC ${statistics_src})
set_target_properties(statistics PROPERTIES POSITION_INDEPENDENT_CODE ON)

# array
file(GLOB Arrays ${CMAKE_CURRENT_SOURCE_DIR}/Sources/arrays/*.cpp)
add_library(arrays STATIC ${Arrays})
set_target_properties(arrays PROPERTIES POSITION_INDEPENDENT_CODE ON)

# manipulations
file(GLOB manipulations ${CMAKE_CURRENT_SOURCE_DIR}/Sources/manipulations/*.cpp)
add_library(manipulations STATIC ${manipulations})
set_target_properties(manipulations PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(manipulations PUBLIC fftw3f)

# mir
file(GLOB mir_source ${CMAKE_CURRENT_SOURCE_DIR}/Sources/mir/*.cpp)
add_library(mir STATIC ${mir_source})
set_target_properties(mir PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(Sources/mir/onsetsds~)
target_link_libraries(mir PUBLIC fftw3 onsetsds_tilde)

# ╭──────────────────────────────────────╮
# │           Library Sources            │
# ╰──────────────────────────────────────╯
list(APPEND PD_NEIMOG "${CMAKE_CURRENT_SOURCE_DIR}/Sources/neimog.cpp")
file(GLOB NEIMOG_ABS ${CMAKE_CURRENT_SOURCE_DIR}/Abstractions/*)
file(GLOB NEIMOG_HELP ${CMAKE_CURRENT_SOURCE_DIR}/Help-Patches/*)
file(GLOB NEIMOG_LUA ${CMAKE_CURRENT_SOURCE_DIR}/Sources/lua/*.pd_lua)
file(GLOB NEIMOG_LUA
     "${NEIMOG_LUA};${CMAKE_CURRENT_SOURCE_DIR}/Sources/lua/*.pd_lua")

pd_add_external(neimog "${PD_NEIMOG}")
pd_add_datafile(neimog "${NEIMOG_ABS}")
pd_add_datafile(neimog "${NEIMOG_HELP}")
pd_add_datafile(neimog "${CMAKE_CURRENT_SOURCE_DIR}/Sources/lua" )
pd_add_datafile(neimog "${NEIMOG_LUA}")

target_link_libraries(neimog statistics arrays manipulations mir)
set_target_properties(neimog PROPERTIES CXX_STANDARD 20)
