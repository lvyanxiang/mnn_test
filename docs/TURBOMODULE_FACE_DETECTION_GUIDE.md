# React Native TurboModule 人脸识别完整实现指南

## 📋 目录

- [项目概述](#项目概述)
- [技术栈](#技术栈)
- [架构设计](#架构设计)
- [环境准备](#环境准备)
- [项目初始化](#项目初始化)
- [共享C++代码实现](#共享c代码实现)
- [Android实现](#android实现)
- [iOS实现](#ios实现)
- [TurboModule集成](#turbomodule集成)
- [测试验证](#测试验证)
- [常见问题](#常见问题)

---

## 项目概述

本项目展示如何在React Native中使用**TurboModule（New Architecture）**实现跨平台人脸识别功能，使用以下技术：

- ✅ **MNN（Mobile Neural Network）** - 阿里巴巴开源的轻量级推理引擎
- ✅ **OpenCV** - 图像处理
- ✅ **UltraFace模型** - 轻量级人脸检测模型
- ✅ **TurboModule** - React Native新架构，零拷贝JSI调用

### 核心特性

- 🎯 **零拷贝调用** - 通过JSI直接调用C++，无Bridge序列化开销
- 🔄 **代码复用** - iOS和Android共用同一套C++实现
- 📦 **自动模型管理** - 模型打包在应用中，自动提取到私有目录
- 🔒 **类型安全** - 通过Codegen生成类型安全的TypeScript接口

---

## 技术栈

| 层级 | 技术 | 说明 |
|------|------|------|
| **JS层** | TypeScript + TurboModule | 类型安全的模块接口 |
| **C++层** | C++17 + JSI | 核心业务逻辑，跨平台共享 |
| **推理引擎** | MNN | 高效神经网络推理 |
| **图像处理** | OpenCV | 图像读取和预处理 |
| **Android桥接** | JNI + Kotlin | Java/C++通信 |
| **iOS桥接** | Objective-C++ + Swift | ObjC/C++/Swift互操作 |

---

## 架构设计

### 整体架构图

```
┌─────────────────────────────────────────────────────┐
│                  React Native App                    │
│                  (TypeScript/JavaScript)             │
└────────────────────┬────────────────────────────────┘
                     │ JSI (零拷贝)
                     ↓
┌─────────────────────────────────────────────────────┐
│            TurboModule Spec (TypeScript)             │
│         NativeSampleModule.ts                      │
└────────────────────┬────────────────────────────────┘
                     │ Codegen
                     ↓
┌─────────────────────────────────────────────────────┐
│         C++ TurboModule Implementation               │
│         NativeSampleModule.cpp/h                   │
│         ┌───────────────────────────────┐            │
│         │   NativeFaceDetector.cpp/h   │            │
│         │   (跨平台共享)                │            │
│         └───────────────────────────────┘            │
└───────────┬───────────────────────┬───────────────────┘
            │ JNI (Android)          │ Objective-C++ (iOS)
            ↓                        ↓
┌───────────────────────┐   ┌───────────────────────────┐
│   ModelExtractor.kt   │   │   iOSModelLoader.mm      │
│   (模型路径管理)      │   │   (模型路径管理)          │
└───────────┬───────────┘   └───────────┬───────────────┘
            │                           │
            ↓                           ↓
┌───────────────────────┐   ┌───────────────────────────┐
│   Android Assets      │   │   iOS Bundle Resources    │
│   RFB-320.mnn         │   │   RFB-320.mnn             │
└───────────────────────┘   └───────────────────────────┘
```

### 目录结构

```
mnn_test/
├── android/                          # Android原生代码
│   └── app/
│       ├── src/main/
│       │   ├── assets/
│       │   │   ├── RFB-320.mnn         # 人脸检测模型
│       │   │   └── det_10g.mnn         # 备用模型
│       │   ├── java/com/anonymous/test_mnn/
│       │   │   └── ModelExtractor.kt     # 模型路径管理
│       │   ├── jni/                    # JNI本地库目录
│       │   │   ├── opencv-mobile-4.13.0-android-nomp/  # OpenCV静态库
│       │   │   │   └── sdk/native/staticlibs/arm64-v8a/
│       │   │   │       └── libopencv_core.a
│       │   │   ├── CMakeLists.txt      # C++编译配置
│       │   │   └── ModelJni.cpp         # JNI桥接
│       │   └── jniLibs/
│       │       └── arm64-v8a/
│       │           └── libMNN.so        # MNN动态库
│       └── build.gradle                # Gradle配置
├── ios/                              # iOS原生代码
│   ├── RFB-320.mnn                  # 人脸检测模型
│   ├── iOSModelLoader.mm/h          # 模型路径管理
│   ├── iOSModelLoader.h             # Objective-C接口
│   └── libs/                        # 第三方库
│       ├── libMNN.a                 # MNN静态库
│       ├── opencv2.framework        # OpenCV框架
│       └── MNN/                     # MNN头文件
├── shared/                           # 跨平台C++代码
│   ├── NativeFaceDetector.h/cpp     # 人脸检测核心实现
│   ├── NativeSampleModule.h/cpp     # TurboModule实现
│   └── ModelJni.h                   # JNI接口声明
├── specs/                           # TypeScript Spec
│   └── NativeSampleModule.ts        # Codegen输入
└── app/                             # React Native代码
```

### 重要说明

**Android OpenCV配置**：
- ✅ **本地静态库**：OpenCV以`.a`静态库形式集成
- ✅ **位置**：`android/app/src/main/jni/opencv-mobile-xxx/sdk/native/staticlibs/arm64-v8a/`
- ✅ **CMakeLists.txt配置**：需要在CMakeLists.txt中添加OpenCV库路径
- ✅ **无需Gradle依赖**：不是通过Gradle远程依赖，而是本地集成

```cmake
# android/app/src/main/cpp/CMakeLists.txt
include_directories(
    ${CMAKE_SOURCE_DIR}/jni/opencv-mobile-xxx/sdk/native/jni/include
)

add_library(opencv_core STATIC IMPORTED
    ${CMAKE_SOURCE_DIR}/jni/opencv-mobile-xxx/sdk/native/staticlibs/arm64-v8a/libopencv_core.a
    ${CMAKE_SOURCE_DIR}/jni/opencv-mobile-xxx/sdk/native/staticlibs/arm64-v8a/libopencv_imgproc.a
)
```

**iOS OpenCV配置**：
- ✅ **手动配置**：需要下载opencv2.framework并添加到项目
- ✅ **路径**：`ios/libs/opencv2.framework`

---

## 环境准备

### 必需工具

| 工具 | 版本要求 | 用途 |
|------|---------|------|
| Node.js | ≥18 | React Native开发 |
| React Native | 0.81+ | 支持New Architecture |
| Xcode | ≥15 | iOS开发 |
| Android Studio | ≥2023 | Android开发 |
| CocoaPods | 最新 | iOS依赖管理 |

### 第三方库准备

#### 1. MNN（跨平台推理引擎）

**Android:**
```bash
# 下载或编译libMNN.so
# 放置到: android/app/src/main/jniLibs/arm64-v8a/
```

**iOS:**
```bash
# 编译静态库
cd MNN/
mkdir build && cd build
cmake .. -DMNN_BUILD_TRAIN=OFF -DMNN_BUILD_CONVERTER=OFF \
  -DCMAKE_TOOLCHAIN_FILE=../toolchains/ios.cmake \
  -DIOS_PLATFORM=OS64
make
# 产物: libMNN.a
```

#### 2. OpenCV

**Android (本地静态库集成):**

opencv-mobile 以静态库形式集成，无需 Gradle 依赖：

1. 下载 opencv-mobile Android 版本
2. 解压到 `android/app/src/main/jni/opencv-mobile-xxx/`
3. 在 CMakeLists.txt 中配置（见上方目录结构说明）

```bash
# 目录结构示例
android/app/src/main/jni/
└── opencv-mobile-4.13.0-android-nomp/
    └── sdk/native/
        ├── jni/include/opencv2/    # 头文件
        └── staticlibs/arm64-v8a/   # 静态库
            ├── libopencv_core.a
            ├── libopencv_imgproc.a
            └── libopencv_imgcodecs.a
```

**iOS:**
```bash
# 下载预编译framework
# 下载地址: https://github.com/nihui/opencv-mobile/releases
# 放置到: ios/libs/
```

#### 3. UltraFace模型

```bash
# 下载RFB-320模型
# 下载地址: https://github.com/Linzaer/Ultra-Light-Fast-Generic-Face-Detector-1MB
# 放置到:
#   - android/app/src/main/assets/RFB-320.mnn
#   - ios/RFB-320.mnn
```

---

## 项目初始化

### 1. 创建Expo项目（支持New Architecture）

```bash
npx create-expo-app@latest --template blank-typescript mnn_test
cd mnn_test

# 启用New Architecture
npx expo prebuild --clean
```

### 2. 配置Codegen

创建 `codegen.config.js`:

```javascript
const path = require('path');

module.exports = {
  serverUrl: 'http://localhost:8081/',
  // 自动生成代码到项目根目录的 generated/
  outputDir: path.resolve(__dirname, 'generated'),
  packageName: 'AppSpecs',
};
```

### 3. 创建TurboModule Spec

创建 `specs/NativeSampleModule.ts`:

```typescript
import {TurboModule, TurboModuleRegistry} from 'react-native';

export interface Spec extends TurboModule {
  readonly reverseString: (input: string) => string;
  readonly addNumbers: (a: number, b: number) => number;
  readonly initFaceDetector: () => string;
  readonly detectFace: (imagePath: string) => string;
}

export default TurboModuleRegistry.getEnforcing<Spec>(
  'NativeSampleModule',
);
```

---

## 共享C++代码实现

### 1. 创建人脸检测器头文件

创建 `shared/NativeFaceDetector.h`:

```cpp
#pragma once

#include <opencv2/opencv.hpp>
#include <MNN/Interpreter.hpp>
#include <MNN/ImageProcess.hpp>
#include <vector>
#include <memory>
#include <string>

namespace facebook::react {

// 人脸信息结构
struct FaceInfo {
    float x;        // 人脸框左上角 x
    float y;        // 人脸框左上角 y
    float width;    // 人脸框宽度
    float height;   // 人脸框高度
    float score;    // 置信度

    FaceInfo() : x(0), y(0), width(0), height(0), score(0) {}
};

class NativeFaceDetector {
public:
    NativeFaceDetector();
    ~NativeFaceDetector();

    // 初始化模型
    int init(const std::string& modelPath);

    // 检测人脸
    int detect(const cv::Mat& img, std::vector<FaceInfo>* faces);

private:
    bool initialized_;
    std::shared_ptr<MNN::Interpreter> interpreter_;
    MNN::Session* session_;
    MNN::Tensor* inputTensor_;
    std::shared_ptr<MNN::CV::ImageProcess> pretreat_;

    // 模型参数（UltraFace RFB-320）
    const int inputSizeWidth_ = 320;
    const int inputSizeHeight_ = 240;
    const float meanVals_[3] = {127.0f, 127.0f, 127.0f};
    const float normVals_[3] = {1.0f / 128.0f, 1.0f / 128.0f, 1.0f / 128.0f};
    const float scoreThreshold_ = 0.95f;
    const float iouThreshold_ = 0.3f;

    // 生成anchors
    int generateAnchors(const int& width, const int& height,
                       const std::vector<std::vector<float>>& minBoxes,
                       const std::vector<float>& strides,
                       std::vector<std::vector<float>>* anchors);

    // NMS 去重
    void nms(const std::vector<FaceInfo>& inputs, std::vector<FaceInfo>* result,
             const float& threshold);

    std::vector<std::vector<float>> anchors_;
};

} // namespace facebook::react
```

### 2. 实现人脸检测器

创建 `shared/NativeFaceDetector.cpp`（完整实现见项目文件）。

**关键点：**
- ✅ 使用条件编译处理平台特定代码
- ✅ 跨平台日志系统
- ✅ MNN模型加载和推理
- ✅ OpenCV图像预处理
- ✅ NMS后处理

---

## Android实现

### 1. 创建JNI桥接

创建 `shared/ModelJni.h`:

```cpp
#pragma once

#ifdef __ANDROID__

#include <jni.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

// 设置模型路径（从Java调用）
void setModelPath(JNIEnv* env, jobject thiz, jstring path);

// 获取模型路径（C++调用）
const char* getModelPath();

#ifdef __cplusplus
}
#endif

#endif // __ANDROID__
```

### 2. 实现JNI桥接

创建 `android/app/src/main/cpp/ModelJni.cpp`:

```cpp
#include "ModelJni.h"
#include <jni.h>
#include <string>
#include <mutex>

static std::string g_model_path;
static std::mutex g_mutex;

extern "C" {

JNIEXPORT void JNICALL
Java_com_anonymous_test_1mnn_ModelExtractor_nativeSetModelPath(
    JNIEnv* env, jobject thiz, jstring path) {
    const char* c_path = env->GetStringUTFChars(path, nullptr);

    std::lock_guard<std::mutex> lock(g_mutex);
    g_model_path = c_path;

    env->ReleaseStringUTFChars(path, c_path);
}

const char* getModelPath() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_model_path.c_str();
}

} // extern "C"
```

### 3. 创建模型路径管理类

创建 `android/app/src/main/java/com/anonymous/test_mnn/ModelExtractor.kt`:

```kotlin
package com.anonymous.test_mnn

import android.content.Context
import android.util.Log
import java.io.File
import java.io.FileOutputStream
import java.io.InputStream

object ModelExtractor {
    private const val TAG = "ModelExtractor"
    private const val MODEL_NAME = "RFB-320.mnn"

    init {
        System.loadLibrary("appmodules")
    }

    fun extractModelIfNeeded(context: Context): String {
        val cacheDir = context.cacheDir
        val modelFile = File(cacheDir, MODEL_NAME)

        if (modelFile.exists()) {
            return modelFile.absolutePath
        }

        try {
            val inputStream: InputStream = context.assets.open(MODEL_NAME)
            val outputStream = FileOutputStream(modelFile)

            val buffer = ByteArray(8192)
            var bytesRead: Int
            while (inputStream.read(buffer).also { bytesRead = it } != -1) {
                outputStream.write(buffer, 0, bytesRead)
            }

            outputStream.close()
            inputStream.close()

            return modelFile.absolutePath
        } catch (e: Exception) {
            Log.e(TAG, "Failed to extract model", e)
            return ""
        }
    }

    fun getModelPath(context: Context): String {
        val path = extractModelIfNeeded(context)
        if (path.isNotEmpty()) {
            nativeSetModelPath(path)
        }
        return path
    }

    private external fun nativeSetModelPath(modelPath: String)
}
```

### 4. 配置CMake

创建 `android/app/src/main/cpp/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.13.1)

project("appmodules")

set(CMAKE_CXX_STANDARD 17)

# MNN库路径
set(MNN_LIB_DIR ${CMAKE_SOURCE_DIR}/../../../libs)

# OpenCV路径
set(OPENCV_DIR ${CMAKE_SOURCE_DIR}/jni/opencv-mobile-4.13.0-android-nomp/sdk/native)

include_directories(
    ${CMAKE_SOURCE_DIR}/../../../../shared
    ${MNN_LIB_DIR}/include
    ${OPENCV_DIR}/jni/include
)

# OpenCV 静态库
add_library(opencv_core STATIC IMPORTED)
set_target_properties(opencv_core PROPERTIES
    IMPORTED_LOCATION ${OPENCV_DIR}/staticlibs/arm64-v8a/libopencv_core.a
)

add_library(opencv_imgproc STATIC IMPORTED)
set_target_properties(opencv_imgproc PROPERTIES
    IMPORTED_LOCATION ${OPENCV_DIR}/staticlibs/arm64-v8a/libopencv_imgproc.a
)

add_library(opencv_imgcodecs STATIC IMPORTED)
set_target_properties(opencv_imgcodecs PROPERTIES
    IMPORTED_LOCATION ${OPENCV_DIR}/staticlibs/arm64-v8a/libopencv_imgcodecs.a
)

add_library(
    appmodules
    SHARED
    NativeFaceDetector.cpp
    NativeSampleModule.cpp
    ModelJni.cpp
)

target_link_libraries(
    appmodules
    log
    android
    opencv_core
    opencv_imgproc
    opencv_imgcodecs
    ${MNN_LIB_DIR}/arm64-v8a/libMNN.so
)
```

### 5. 配置build.gradle

```gradle
android {
    // ...其他配置

    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
        }
    }
}

// 注意: OpenCV 通过本地静态库集成，无需 Gradle 依赖
// OpenCV 静态库位于: jni/opencv-mobile-xxx/sdk/native/staticlibs/
```

---

## iOS实现

### 1. 创建模型加载器

创建 `ios/iOSModelLoader.h`:

```objc
#pragma once

#import <Foundation/Foundation.h>

@interface iOSModelLoader : NSObject

+ (void)setModelPath:(NSString *)modelPath;
+ (NSString *)getModelPath;

@end

// C++ 接口（供 C++ 代码调用）
#ifdef __cplusplus
extern "C" {
    const char* getIOSModelPath();
}
#endif
```

### 2. 实现模型加载器

创建 `ios/iOSModelLoader.mm`:

```objc
#import "iOSModelLoader.h"
#import <Foundation/Foundation.h>

static NSString* g_model_path = nil;

@implementation iOSModelLoader

+ (void)setModelPath:(NSString *)modelPath {
    if (modelPath) {
        g_model_path = [modelPath copy];
        NSLog(@"[iOSModelLoader] Model path set: %@", g_model_path);
    }
}

+ (NSString *)getModelPath {
    return g_model_path;
}

@end

extern "C" {
    const char* getIOSModelPath() {
        if (g_model_path) {
            return [g_model_path UTF8String];
        }
        return nullptr;
    }
}
```

### 3. 在AppDelegate中初始化

修改 `ios/testmnn/AppDelegate.swift`:

```swift
import UIKit

@UIApplicationMain
class AppDelegate: ExpoAppDelegate {

  var window: UIWindow?

  override func application(
    _ application: UIApplication,
    didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
  ) -> Bool {

    // 初始化模型路径
    if let modelPath = Bundle.main.path(forResource: "RFB-320", ofType: "mnn") {
      NSLog("[AppDelegate] Model found at: \(modelPath)")
      iOSModelLoader.setModelPath(modelPath)
    } else {
      NSLog("[AppDelegate] ERROR: Model file not found!")
    }

    // ...其他初始化代码
    return super.application(application, didFinishLaunchingWithOptions: launchOptions)
  }
}
```

### 4. 配置Xcode项目

#### 添加库和框架

1. **MNN静态库**:
   - 路径: `ios/libs/libMNN.a`
   - 添加到: Build Phases → Link Binary With Libraries

2. **OpenCV框架**:
   - 路径: `ios/libs/opencv2.framework`
   - 添加到: Build Phases → Link Binary With Libraries

3. **模型文件**:
   - 路径: `ios/RFB-320.mnn`
   - 添加到: Build Phases → Copy Bundle Resources

#### 配置Header Search Paths

```swift
// Build Settings → HEADER_SEARCH_PATHS
$(PROJECT_DIR)/libs
$(PROJECT_DIR)/shared
```

#### 配置Framework Search Paths

```swift
// Build Settings → FRAMEWORK_SEARCH_PATHS
$(PROJECT_DIR)/libs
```

---

## TurboModule集成

### 1. 创建TurboModule头文件

创建 `shared/NativeSampleModule.h`:

```cpp
#pragma once

#include <AppSpecsJSI.h>
#include <jsi/jsi.h>
#include <memory>
#include "NativeFaceDetector.h"

namespace facebook::react {

class NativeSampleModule : public NativeSampleModuleCxxSpec<NativeSampleModule> {
public:
  NativeSampleModule(std::shared_ptr<CallInvoker> jsInvoker);
  ~NativeSampleModule();

  jsi::String reverseString(jsi::Runtime& rt, jsi::String input);
  double addNumbers(jsi::Runtime& rt, double a, double b);
  jsi::String initFaceDetector(jsi::Runtime& rt);
  jsi::String detectFace(jsi::Runtime& rt, jsi::String imagePath);

private:
  std::unique_ptr<NativeFaceDetector> faceDetector_;
  bool detectorInitialized_;
};

} // namespace facebook::react
```

### 2. 实现TurboModule

创建 `shared/NativeSampleModule.cpp`:

```cpp
#include "NativeSampleModule.h"
#include <fstream>

// 平台特定的头文件和日志宏
#ifdef __ANDROID__
  #include "ModelJni.h"
  #include <android/log.h>
  #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
  #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#else
  #include <cstdio>
  extern "C" {
    const char* getIOSModelPath(void);
  }
  #define LOGI(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
  #define LOGE(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#endif

#define TAG "NativeSampleModule"

namespace facebook::react {

NativeSampleModule::NativeSampleModule(std::shared_ptr<CallInvoker> jsInvoker)
    : NativeSampleModuleCxxSpec(std::move(jsInvoker))
    , detectorInitialized_(false) {
  faceDetector_ = std::make_unique<NativeFaceDetector>();
  LOGI("NativeSampleModule created");
}

NativeSampleModule::~NativeSampleModule() {
  LOGI("NativeSampleModule destroyed");
}

// 示例方法
jsi::String NativeSampleModule::reverseString(jsi::Runtime& rt, jsi::String input) {
  std::string inputStr = input.utf8(rt);
  std::string reversed(inputStr.rbegin(), inputStr.rend());
  return jsi::String::createFromUtf8(rt, reversed);
}

double NativeSampleModule::addNumbers(jsi::Runtime&, double a, double b) {
  return a + b;
}

// 初始化人脸检测器
jsi::String NativeSampleModule::initFaceDetector(jsi::Runtime& rt) {
#ifdef __ANDROID__
  const char* modelPath = getModelPath();
#else
  const char* modelPath = getIOSModelPath();
#endif

  if (!modelPath) {
    return jsi::String::createFromUtf8(rt,
      "{\"error\":\"Model path not available\"}");
  }

  std::string pathStr(modelPath);
  int ret = faceDetector_->init(pathStr);

  if (ret != 0) {
    return jsi::String::createFromUtf8(rt,
      "{\"error\":\"Failed to initialize\",\"code\":" + std::to_string(ret) + "}");
  }

  detectorInitialized_ = true;
  return jsi::String::createFromUtf8(rt,
    "{\"status\":\"success\",\"message\":\"Detector initialized\"}");
}

// 检测人脸
jsi::String NativeSampleModule::detectFace(jsi::Runtime& rt, jsi::String imagePath) {
  if (!detectorInitialized_) {
    return jsi::String::createFromUtf8(rt,
      "{\"error\":\"Detector not initialized\"}");
  }

  std::string pathStr = imagePath.utf8(rt);
  cv::Mat image = cv::imread(pathStr);

  if (image.empty()) {
    return jsi::String::createFromUtf8(rt,
      "{\"error\":\"Failed to read image\"}");
  }

  std::vector<FaceInfo> faces;
  int ret = faceDetector_->detect(image, &faces);

  if (ret != 0) {
    return jsi::String::createFromUtf8(rt,
      "{\"error\":\"Detection failed\"}");
  }

  // 构建JSON结果
  std::string json = "{\"faces\":[";
  for (size_t i = 0; i < faces.size(); i++) {
    json += "{";
    json += "\"x\":" + std::to_string((int)faces[i].x) + ",";
    json += "\"y\":" + std::to_string((int)faces[i].y) + ",";
    json += "\"width\":" + std::to_string((int)faces[i].width) + ",";
    json += "\"height\":" + std::to_string((int)faces[i].height) + ",";
    json += "\"score\":" + std::to_string(faces[i].score);
    json += "}";
    if (i < faces.size() - 1) json += ",";
  }
  json += "]}";

  return jsi::String::createFromUtf8(rt, json);
}

} // namespace facebook::react
```

---

## 测试验证

### JavaScript调用示例

```javascript
import NativeSampleModule from './specs/NativeSampleModule';

// 测试基础方法
console.log(await NativeSampleModule.reverseString("Hello")); // "olleH"
console.log(await NativeSampleModule.addNumbers(3, 5));    // 8

// 初始化人脸检测器
const initResult = await NativeSampleModule.initFaceDetector();
console.log(initResult);
// {"status":"success","message":"Detector initialized"}

// 检测人脸
const imagePath = "/path/to/face.jpg";
const result = await NativeSampleModule.detectFace(imagePath);
console.log(result);
// {"faces":[{"x":100,"y":150,"width":200,"height":250,"score":0.98}]}
```

### 预期输出

```
[INFO] NativeSampleModule created
[INFO] initFaceDetector called
[INFO] Model path: /path/to/RFB-320.mnn
[INFO] Face detector initialized successfully
[INFO] detectFace called with path: /path/to/face.jpg
[INFO] Image loaded: 640x480
[INFO] Detection result: 1 faces detected
```

---

## 常见问题

### Q1: 编译错误 "undefined symbol"

**问题**: 链接时找不到符号定义

**解决方案**:
- 检查是否所有 `.cpp` 文件都添加到了编译源
- iOS: 检查 Xcode 的 Build Phases → Compile Sources
- Android: 检查 CMakeLists.txt 是否包含所有源文件

### Q2: 运行时错误 "Model path not available"

**问题**: 模型路径未正确初始化

**解决方案**:
- **Android**: 确认 `ModelExtractor.getModelPath()` 在 `initFaceDetector()` 之前调用
- **iOS**: 确认 `AppDelegate.swift` 中调用了 `iOSModelLoader.setModelPath()`
- 检查模型文件是否正确打包到 assets/bundle

### Q3: "Raw string literal" 编译错误

**问题**: 原始字符串语法错误

**原因**: C++原始字符串 `R"(content)"` 中的 `)` 会提前结束字符串

**解决方案**:
```cpp
// ❌ 错误 - 内容中包含括号
R"({"message":"Detector initialized (iOS)"})";

// ✅ 正确 - 使用转义字符
"{\"message\":\"Detector initialized (iOS)\"}";
```

### Q4: iOS找不到Foundation框架

**问题**: iOS编译时找不到 Foundation 头文件

**解决方案**:
- 确保文件扩展名是 `.mm` (Objective-C++) 而不是 `.cpp`
- 在 `.cpp` 文件中不要包含 Foundation 头文件

### Q5: Android找不到opencv2/opencv.hpp

**问题**: OpenCV头文件找不到

**解决方案**:
```gradle
// 确保添加了opencv-mobile依赖
implementation 'com.github.nihui:opencv-mobile:4.8.0-alpha1'

// 或者在CMakeLists.txt中添加OpenCV路径
include_directories(/path/to/opencv/include)
```

---

## 性能优化建议

### 1. 模型初始化

✅ **在应用启动时初始化一次**
```javascript
// App启动时调用
useEffect(() => {
  NativeSampleModule.initFaceDetector();
}, []);
```

❌ **避免每次检测前重新初始化**
```javascript
// 不要这样做
await NativeSampleModule.initFaceDetector(); // 每次
await NativeSampleModule.detectFace(path);
```

### 2. 图像预处理

✅ **在C++层进行图像缩放**
```cpp
// 将图像缩放到模型输入尺寸
cv::resize(image, image, cv::Size(320, 240));
```

### 3. 内存管理

✅ **及时释放资源**
```cpp
NativeFaceDetector::~NativeFaceDetector() {
    if (session_) {
        interpreter_->releaseSession(session_);
        session_ = nullptr;
    }
}
```

---

## 总结

通过本指南，你学会了：

1. ✅ **创建跨平台TurboModule**
2. ✅ **集成MNN和OpenCV**
3. ✅ **实现模型自动加载**
4. ✅ **处理JNI和Objective-C++桥接**
5. ✅ **编写跨平台C++代码**

### 关键要点

- 🔑 **条件编译** - 使用 `#ifdef __ANDROID__` 分离平台代码
- 🔑 **类型安全** - Codegen生成TS接口
- 🔑 **零拷贝** - JSI直接调用，无序列化
- 🔑 **代码复用** - 共享C++实现

### 扩展建议

- [ ] 添加更多人脸识别功能（关键点检测、人脸识别等）
- [ ] 支持GPU加速（MNN GPU、Metal、Vulkan）
- [ ] 添加模型版本管理和热更新
- [ ] 实现多模型管理
- [ ] 添加性能监控和统计

---

**文档版本**: 1.0
**最后更新**: 2026-02-10
**适用版本**: React Native 0.81+
