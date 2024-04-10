cmake_minimum_required(VERSION 3.15)
include(${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.build/pd.cmake) # pd.build
project(neimog)

cmake_policy(SET CMP0077 NEW)

#╭──────────────────────────────────────╮
#│           Another objects            │
#╰──────────────────────────────────────╯

# ───────────── musesampler ─────────────
set(MUSE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Externals/pd-musesampler)

if (NOT DEFINED MUSESCORE_VERSION)
    set(MUSESCORE_VERSION "v4.2.1")
    message(STATUS "Using apitypes.h from Musescore version: ${MUSESCORE_VERSION}")
endif()
set(MUSESCORE_API_TYPES ${MUSE_ROOT}/Sources/Musescore/apitypes.h)

if (NOT EXISTS ${MUSESCORE_API_TYPES})
    message(STATUS "Downloading Musescore API types")
    if (NOT EXISTS ${MUSE_ROOT}/Sources/Musescore)
        message(STATUS "Creating Musescore directory")
        file(MAKE_DIRECTORY ${MUSE_ROOT}/Sources/Musescore)
    endif()
    file(DOWNLOAD https://raw.githubusercontent.com/musescore/MuseScore/${MUSESCORE_VERSION}/src/framework/musesampler/internal/apitypes.h ${MUSESCORE_API_TYPES})

    # Remove #include "log.h"
    file(READ ${MUSESCORE_API_TYPES} MUSESCORE_API_TYPES_CONTENTS)
    string(REGEX REPLACE "#include[ \t]+\"log.h\"" "// #include \"log.h\"" MUSESCORE_API_TYPES_CONTENTS "${MUSESCORE_API_TYPES_CONTENTS}")
    file(WRITE ${MUSESCORE_API_TYPES} "${MUSESCORE_API_TYPES_CONTENTS}")
endif()

file(GLOB MuseSources ${MUSE_ROOT}/Sources/*.cpp)
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





