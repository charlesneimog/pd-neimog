cmake_minimum_required(VERSION 3.15)
include(${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.cmake/pd.cmake) # pd.build
project(pd-neimog)

cmake_policy(SET CMP0077 NEW)

# ────────────── divergence ──────────────
file(GLOB statistics_src ${CMAKE_CURRENT_SOURCE_DIR}/Sources/statistics/*.cpp)
add_library(statistics STATIC ${statistics_src})
set_target_properties(statistics PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ─────────────── array ───────────────
file(GLOB Arrays ${CMAKE_CURRENT_SOURCE_DIR}/Sources/arrays/*.cpp)
add_library(arrays STATIC ${Arrays})
set_target_properties(arrays PROPERTIES POSITION_INDEPENDENT_CODE ON)

# # ─────────────── muse~ ───────────────
# file(GLOB Arrays ${CMAKE_CURRENT_SOURCE_DIR}/Sources/musesampler~/*.cpp)
# add_library(muse STATIC ${Arrays})
# set_target_properties(muse PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ╭──────────────────────────────────────╮
# │           Library Sources            │
# ╰──────────────────────────────────────╯
list(APPEND PD_NEIMOG "${CMAKE_CURRENT_SOURCE_DIR}/Sources/neimog.cpp")
file(GLOB NEIMOG_ABS ${CMAKE_CURRENT_SOURCE_DIR}/Abstractions/*)
file(GLOB NEIMOG_HELP ${CMAKE_CURRENT_SOURCE_DIR}/Help-Patches/*)

pd_add_external(pd-neimog ${PD_NEIMOG})
pd_add_datafile(pd-neimog "${NEIMOG_ABS}")
pd_add_datafile(pd-neimog "${NEIMOG_HELP}")

target_link_libraries(pd-neimog statistics arrays)
set_target_properties(pd-neimog PROPERTIES CXX_STANDARD 20)



#
