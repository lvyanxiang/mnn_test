cmake_minimum_required(VERSION 3.13)

# Define the library name here.
project(appmodules)

# This file includes all the necessary to let you build your React Native application
include(${REACT_ANDROID_DIR}/cmake-utils/ReactNative-application.cmake)


# ========== 新增：添加 MNN 库 ==========
# 导入预编译的 MNN 库
add_library(mnn SHARED IMPORTED)
set_target_properties(mnn PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libMNN.so)

# 添加 MNN 头文件路径
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# =======================================

# ========== 新增：添加 OpenCV 库 ==========
set(OpenCV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/opencv-mobile-4.13.0-android-nomp/sdk/native/jni)

find_package(OpenCV REQUIRED)

# =======================================

# Define where the additional source code lives. We need to crawl back the jni, main, src, app, android folders
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  ../../../../../shared/NativeSampleModule.cpp
  ../../../../../shared/NativeFaceDetector.cpp
  OnLoad.cpp
  ModelJni.cpp
)

# Define where CMake can find the additional header files. We need to crawl back the jni, main, src, app, android folders
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ../../../../../shared)


# ========== 新增：链接 MNN 和 OpenCV 库 ==========
target_link_libraries(${CMAKE_PROJECT_NAME} mnn ${OpenCV_LIBS})
# =======================================
