cmake_minimum_required(VERSION 3.15)
include(${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.cmake/pd.cmake) # pd.build
project(pd-neimog)

#╭──────────────────────────────────────╮
#│                FFTW3                 │
#╰──────────────────────────────────────╯
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
     ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/)

add_subdirectory(Libraries/fftw-3.3.10 EXCLUDE_FROM_ALL)
set_target_properties(fftw3 PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(fftw3 PROPERTIES POSITION_INDEPENDENT_CODE ON)

#╭──────────────────────────────────────╮
#│               Objects                │
#╰──────────────────────────────────────╯
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
target_link_libraries(manipulations PUBLIC fftw3)

# mir 
file(GLOB mir_source ${CMAKE_CURRENT_SOURCE_DIR}/Sources/mir/*.cpp)
add_library(mir STATIC ${mir_source})
set_target_properties(mir PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(mir PUBLIC fftw3)

# ╭──────────────────────────────────────╮
# │           Library Sources            │
# ╰──────────────────────────────────────╯
list(APPEND PD_NEIMOG "${CMAKE_CURRENT_SOURCE_DIR}/Sources/neimog.cpp")
file(GLOB NEIMOG_ABS ${CMAKE_CURRENT_SOURCE_DIR}/Abstractions/*)
file(GLOB NEIMOG_HELP ${CMAKE_CURRENT_SOURCE_DIR}/Help-Patches/*)
file(GLOB NEIMOG_LUA ${CMAKE_CURRENT_SOURCE_DIR}/Sources/lua/*.pd_lua)

pd_add_external(pd-neimog ${PD_NEIMOG})
pd_add_datafile(pd-neimog "${NEIMOG_ABS}")
pd_add_datafile(pd-neimog "${NEIMOG_HELP}")
pd_add_datafile(pd-neimog "${CMAKE_CURRENT_SOURCE_DIR}/Sources/lua" DESTINATION "lua")
pd_add_datafile(pd-neimog ${NEIMOG_LUA} DESTINATION "lua")

target_link_libraries(pd-neimog statistics arrays manipulations mir)
set_target_properties(pd-neimog PROPERTIES CXX_STANDARD 20)



#
