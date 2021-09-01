cmake_minimum_required(VERSION 3.19)
project(FFmpegPython)

set(CMAKE_CXX_STANDARD 11)

if (CMAKE_HOST_WIN32)
    set(WINDOWS 1)
elseif (CMAKE_HOST_APPLE)
    set(MACOS 1)
elseif (CMAKE_HOST_UNIX)
    set(LINUX 1)
endif ()
set(PROJECT_ROOT_PATH ${CMAKE_SOURCE_DIR})

if (WINDOWS)
    #设置OpenCV库路径
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/3rdparty/opencv451/x64/vc15/lib)
    set(OpenCV_INCLUDES ${CMAKE_SOURCE_DIR}/3rdparty/opencv451/include)
    set(OpenCV_LINK ${CMAKE_SOURCE_DIR}/3rdparty/opencv451/x64/vc15/lib)
    #设置ffmpeg库路径
    set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg432/x64/windows)
    set(FFMPEG_LINK ${FFMPEG_DIR}/lib)
    set(FFMPEG_INCLUDES ${FFMPEG_DIR}/include)
    set(FFMPEG_LIBS avfilter avformat avutil avcodec avdevice swscale postproc)
elseif (LINUX)
    #设置OpenCV库路径
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/3rdparty/opencv451/x64/vc15/lib)
    set(OpenCV_INCLUDES ${CMAKE_SOURCE_DIR}/3rdparty/opencv451/include)
    set(OpenCV_LINK ${CMAKE_SOURCE_DIR}/3rdparty/opencv451/x64/vc15/lib)
    #设置ffmpeg库路径
    set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg432/x64/windows)
    set(FFMPEG_LINK ${FFMPEG_DIR}/lib)
    set(FFMPEG_INCLUDES ${FFMPEG_DIR}/include)
    set(FFMPEG_LIBS avfilter avformat avutil avcodec avdevice swscale postproc)
endif ()

include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.91chifun.workers.dev/https://github.com//google/googletest/archive/refs/tags/release-1.11.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(OpenCV REQUIRED)

link_directories(${FFMPEG_LINK})

include_directories(${FFMPEG_INCLUDES})

IF (CMAKE_BUILD_TYPE STREQUAL Release)
    add_library(${PROJECT_NAME} SHARED library.cpp)
ENDIF ()

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_executable(${PROJECT_NAME} library.cpp test.cpp)
    target_link_libraries(${PROJECT_NAME} gtest_main)
ENDIF ()

target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS}
        ${FFMPEG_LIBS}
        )
if (WINDOWS)
    file(GLOB_RECURSE DEPEND_DLLS
            ${OpenCV_DIR}/x64/vc15/bin/opencv_world451.dll
            ${FFMPEG_DIR}/bin/avcodec-58.dll
            ${FFMPEG_DIR}/bin/avdevice-58.dll
            ${FFMPEG_DIR}/bin/avfilter-7.dll
            ${FFMPEG_DIR}/bin/avformat-58.dll
            ${FFMPEG_DIR}/bin/avutil-56.dll
            ${FFMPEG_DIR}/bin/postproc-55.dll
            ${FFMPEG_DIR}/bin/swscale-5.dll
            ${FFMPEG_DIR}/bin/swresample-3.dll
            )
    file(COPY ${DEPEND_DLLS} DESTINATION .)
endif ()


