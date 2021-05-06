cmake_minimum_required(VERSION 3.4.1)

include_directories(include)
include_directories(soundtouch/include)
include_directories(soundtouch/SoundTouch)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")

aux_source_directory(. SOURCE)
aux_source_directory(./soundtouch/SoundTouch SOURCE1)

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${SOURCE}
        ${SOURCE1}
        )

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

target_link_libraries( # Specifies the target library.
        native-lib
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swresample
        swscale
        android
        OpenSLES
        ${log-lib})