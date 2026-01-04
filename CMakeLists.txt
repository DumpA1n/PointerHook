cmake_minimum_required(VERSION 3.22.1)

if(WIN32 OR APPLE)
    set(CMAKE_SYSTEM_NAME Android)
    set(CMAKE_SYSTEM_VERSION 27)
    set(ANDROID_PLATFORM 27)
    set(ANDROID_ABI arm64-v8a)
    if(WIN32)
        set(ANDROID_NDK "C:/Users/DDD/AppData/Local/Android/Sdk/ndk/android-ndk-r27c")
    elseif(APPLE)
        set(ANDROID_NDK "/Users/dj/android-ndk-r27d")
    endif()
    set(CMAKE_TOOLCHAIN_FILE "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
endif()

project(ptrhook C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-w)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-fvisibility=hidden -ffunction-sections -fdata-sections)
    add_link_options(-Wl,--gc-sections -Wl,--strip-all -Wl,--exclude-libs,ALL)
endif()

# TODO: Remove xHook dependency
file(GLOB XHOOK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/xHook/*.c
)

file(GLOB KITTY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/KittyMemory/*.cpp
)

add_library(${PROJECT_NAME} SHARED 
    ${CMAKE_CURRENT_SOURCE_DIR}/example.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IPointerHook.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trampoline.s
    ${XHOOK_SOURCES}
    ${KITTY_SOURCES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    kNO_KEYSTONE
    kANDROID_LOG
    kANDROID_LOG_TAG="INJECT"
    kPROJECT_NAME="${PROJECT_NAME}"
)

target_link_libraries(${PROJECT_NAME}
    android
    log
)
