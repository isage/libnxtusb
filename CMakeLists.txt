cmake_minimum_required(VERSION 2.6)
include(CheckCXXCompilerFlag)

project(libnxtusb)

set (libnxtusb_VERSION_MAJOR 1)
set (libnxtusb_VERSION_MINOR 0)

option(BUILD_SHARED_LIBS "Build Shared Libraries" ON)

file(GLOB sources "[a-zA-Z]*.c")
file(GLOB private_headers "[a-zA-Z]*.h")


find_package(PkgConfig)
pkg_check_modules(PC_LIBUSB QUIET libusb-1.0)
set(LIBUSB_DEFINITIONS ${PC_LIBUSB_CFLAGS_OTHER})

find_path(LIBUSB_INCLUDE_DIR libusb-1.0/libusb.h
  HINTS ${PC_LIBUSB_INCLUDEDIR} ${PC_LIBUSB_INCLUDE_DIRS}
    PATH_SUFFIXES libusb-1.0 )

find_library(LIBUSB_LIBRARY NAMES usb-1.0 libusb-1.0
     HINTS ${PC_LIBUSB_LIBDIR} ${PC_LIBUSB_LIBRARY_DIRS} )

set(LIBUSB_LIBRARIES ${LIBUSB_LIBRARY} )
set(LIBUSB_INCLUDE_DIRS ${LIBUSB_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUsb  DEFAULT_MSG
    LIBUSB_LIBRARY LIBUSB_INCLUDE_DIR)

mark_as_advanced(LIBUSB_INCLUDE_DIR LIBUSB_LIBRARY )

message(${libnxtusb_SOURCE_DIR})
include_directories(${libnxtusb_SOURCE_DIR})
include_directories(${LIBUSB_INCLUDE_DIR})

add_library(nxtusb ${sources})

target_link_libraries(nxtusb ${LIBUSB_LIBRARY})

add_subdirectory(example)

