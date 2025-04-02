cmake_minimum_required(VERSION 3.25)
project(onsetsds~)

set(PDCMAKE_FILE ${CMAKE_BINARY_DIR}/pd.cmake)
if(NOT EXISTS ${PDCMAKE_FILE})
  message(STATUS "Downloading pd.cmake")
  file(
    DOWNLOAD
    https://raw.githubusercontent.com/pure-data/pd.cmake/refs/tags/v0.1.0/pd.cmake
    ${PDCMAKE_FILE}
    SHOW_PROGRESS
    STATUS DOWNLOAD_STATUS)
endif()
include(${PDCMAKE_FILE})

file(GLOB ONSETSDS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

pd_add_external(onsetsds~ "${ONSETSDS_SOURCES}" LINK_LIBRARIES fftw3f)
