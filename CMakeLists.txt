# ----------------------------------------------------------------------------
#  Root CMake file for Rockchip Media Process Platform (MPP)
#
#   - 10:34 2015/7/27: Initial version <herman.chen@rock-chips.com>
#
# ----------------------------------------------------------------------------

# vim: syntax=cmake
if(NOT CMAKE_BUILD_TYPE)
    # default to Release build for GCC builds
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()
message(STATUS "cmake version ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 11)

cmake_minimum_required (VERSION 2.8.8) # OBJECT libraries require 2.8.8
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckCXXCompilerFlag)

# Search packages for host system instead of packages for target system
# in case of cross compilation these macro should be defined by toolchain file
if(NOT COMMAND find_host_package)
    macro(find_host_package)
        find_package(${ARGN})
    endmacro()
endif()
if(NOT COMMAND find_host_program)
    macro(find_host_program)
        find_program(${ARGN})
    endmacro()
endif()

project (rk_mpp)

# ----------------------------------------------------------------------------
# enable test in this project
# ----------------------------------------------------------------------------

include_directories(${PROJECT_SOURCE_DIR}/inc)
include_directories(${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR}/base/inc)
include_directories(${PROJECT_SOURCE_DIR}/codec/inc)
include_directories(${PROJECT_SOURCE_DIR}/hal/inc)
include_directories(${PROJECT_SOURCE_DIR}/hal/common)
include_directories(${PROJECT_SOURCE_DIR}/vproc/inc)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/usr/local/ffmpeg/include)



# link_directories(/home/rpdzkj/env/mpp/build/linux/aarch64/self_utils)
# link_directories(/home/rpdzkj/wjm/pinlingv2.3.1/pinlingv2.3/mppEncoder/build/utils)
# link_directories(/home/rpdzkj/env/mpp/build/linux/aarch64/utils)
# link_directories(/usr/local/ffmpeg/lib)




set(module mpi_enc)

set(ext c)
if (NOT MPP_PROJECT_NAME)
    set(MPP_PROJECT_NAME rockchip_mpp)
endif()
set(MPP_SHARED ${MPP_PROJECT_NAME})

set(CMAKE_INSTALL_PREFIX "./install/")

set(SRC_FILES 
utils/camera_source.c
utils/mpi_enc_utils.c
utils/dictionary.c
utils/iniparser.c
utils/mpp_enc_roi_utils.c
utils/mpp_opt.c
utils/utils.c
)

# add_subdirectory(utils)
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})
find_package(ZLIB REQUIRED)



add_executable(mpp_encoder src/main.cpp src/mpp_event_trigger.c src/mpp_parse_cfg.c src/mppEncoder.cpp ${SRC_FILES})
target_link_libraries(mpp_encoder ${MPP_SHARED} ${OpenCV_LIBS} -lz -lavcodec -lavformat -lavutil -lswresample -lswscale -lswresample -lpthread)
# target_link_libraries(mpp_encoder ${MPP_SHARED} ${OpenCV_LIBS} )
