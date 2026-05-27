cmake_minimum_required(VERSION 3.22.1)

if(DEFINED ENV{NDK_HOME})
    set(ANDROID_PLATFORM 21)
    set(ANDROID_ABI arm64-v8a)
    file(TO_CMAKE_PATH "$ENV{NDK_HOME}" NDK_HOME_CMAKE)
    set(CMAKE_TOOLCHAIN_FILE "${NDK_HOME_CMAKE}/build/cmake/android.toolchain.cmake")
else()
    message(FATAL_ERROR "NDK_HOME environment variable not set. Please set it to the path of your Android NDK.")
endif()

project(ptrhook C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-fvisibility=hidden -ffunction-sections -fdata-sections)
    add_link_options(-Wl,--gc-sections -Wl,--strip-all -Wl,--exclude-libs,ALL)
endif()

file(GLOB ASM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ph_arm64_glue.s
    ${CMAKE_CURRENT_SOURCE_DIR}/ph_trampoline_pool.s
)

add_library(${PROJECT_NAME}_asm OBJECT ${ASM_SOURCES})
set_target_properties(${PROJECT_NAME}_asm PROPERTIES EXPORT_COMPILE_COMMANDS OFF)

add_library(${PROJECT_NAME} SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/example.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IPointerHook.cpp
    $<TARGET_OBJECTS:${PROJECT_NAME}_asm>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    kANDROID_LOG_TAG="INJECT"
    kPROJECT_NAME="${PROJECT_NAME}"
)

target_link_libraries(${PROJECT_NAME}
    android
    log
)
