cmake_minimum_required(VERSION 3.15)
include(${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.build/pd.cmake) # pd.build
project(neimog)

cmake_policy(SET CMP0077 NEW)

#╭──────────────────────────────────────╮
#│           Another objects            │
#╰──────────────────────────────────────╯

# ───────────── musesampler ─────────────
file(GLOB MuseSources ${CMAKE_CURRENT_SOURCE_DIR}/Externals/pd-musesampler/Sources/*.cpp)
add_library(musesampler STATIC ${MuseSources})
set_target_properties(musesampler PROPERTIES POSITION_INDEPENDENT_CODE ON)


# ──────────────── ambi~ ────────────────
set(AMBI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Externals/ambi_tilde)

set(BUILD_TESTS OFF)
set(BUILD_STATIC_LIBS ON)
set(MYSOFA_LIBRARIES mysofa-static)
set(BUILD_SHARED_LIBS OFF)
set(MYSOFA_FOUND ON)

add_subdirectory(${AMBI_ROOT}/Libraries/libmysofa/) # sofa include
set_target_properties(mysofa-static PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_definitions(-DHAVE_MYSOFA)
add_subdirectory(${AMBI_ROOT}/Libraries/libspatialaudio)
target_include_directories(spatialaudio-static PUBLIC "${AMBI_ROOT}/Libraries/libmysofa/src/")
target_include_directories(spatialaudio-static PUBLIC "${AMBI_ROOT}/Libraries/libmysofa/src/hrtf")
set_target_properties(spatialaudio-static PROPERTIES POSITION_INDEPENDENT_CODE ON)

file(GLOB AmbiSources "${AMBI_ROOT}/Sources/*.cpp")
add_library(ambi STATIC ${AmbiSources})
set_target_properties(ambi PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(ambi PRIVATE "${AMBI_ROOT}/Libraries/libspatialaudio/include/hrtf/")
target_include_directories(ambi PRIVATE "${AMBI_ROOT}/Libraries/libspatialaudio/include/")
target_include_directories(ambi PRIVATE "${AMBI_ROOT}/Libraries/libspatialaudio/source/kiss_fft/")
target_include_directories(ambi PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/Externals/ambi_tilde/Libraries/libmysofa/src/")
target_link_libraries(ambi spatialaudio-static mysofa-static)

# ─────────── Partial Tracking ───────────
set(PARTIAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Externals/pd-partialtracking)



#╭──────────────────────────────────────╮
#│           Library Sources            │
#╰──────────────────────────────────────╯
set_pd_external_path(${CMAKE_CURRENT_SOURCE_DIR}/)
file(GLOB PD_NEIMOG ${CMAKE_CURRENT_SOURCE_DIR}/Sources/**/*.cpp)
add_pd_external(neimog pd-neimog ${PD_NEIMOG})
set_target_properties(neimog PROPERTIES CXX_STANDARD 20)

target_link_libraries(neimog musesampler ambi)





