cmake_minimum_required(VERSION 2.8)

project(rtl_sdr)

set(CMAKE_CXX_FLAGS "-std=c++1z -O3")
set(CMAKE_CXX_COMPILER "clang++")


add_executable(${PROJECT_NAME} "main.cpp")


# rtl-sdr
if(NOT RTLSDR_FOUND)

    find_package(PkgConfig)
    pkg_check_modules (RTLSDR_PKG librtlsdr)
    set(RTLSDR_DEFINITIONS ${PC_RTLSDR_CFLAGS_OTHER})

    find_path(RTLSDR_INCLUDE_DIR
                NAMES rtl-sdr.h
                HINTS ${RTLSDR_PKG_INCLUDE_DIRS} $ENV{RTLSDR_DIR}/include
                PATHS /usr/local/include /usr/include /opt/include /opt/local/include)

    find_library(RTLSDR_LIBRARY
                NAMES rtlsdr
                HINTS ${RTLSDR_PKG_LIBRARY_DIRS} $ENV{RTLSDR_DIR}/include
                PATHS /usr/local/lib /usr/lib /opt/lib /opt/local/lib)

    set(RTLSDR_LIBRARIES ${RTLSDR_LIBRARY} )
    set(RTLSDR_INCLUDE_DIRS ${RTLSDR_INCLUDE_DIR} )

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set LibRTLSDR_FOUND to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(rtlsdr  DEFAULT_MSG
                                      RTLSDR_LIBRARY RTLSDR_INCLUDE_DIR)

    mark_as_advanced(RTLSDR_INCLUDE_DIR RTLSDR_LIBRARY)

endif(NOT RTLSDR_FOUND)
target_link_libraries(${PROJECT_NAME} ${RTLSDR_LIBRARIES})
