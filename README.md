# MNN 人脸识别 Demo

基于 React Native TurboModule（新架构）的跨平台人脸识别应用，使用 MNN 推理引擎和 UltraFace 模型。

## 项目概述

本项目展示了如何在 React Native 中使用 **TurboModule（New Architecture）** 实现跨平台人脸识别功能。

### 核心技术

- **MNN** - 阿里巴巴开源的轻量级深度学习推理引擎
- **OpenCV** - 图像处理库
- **UltraFace (RFB-320)** - 轻量级人脸检测模型（约1MB）
- **TurboModule** - React Native 新架构，通过 JSI 实现零拷贝调用

### 架构特点

- 零拷贝调用：通过 JSI 直接调用 C++，无 Bridge 序列化开销
- 代码复用：iOS 和 Android 共用同一套 C++ 实现
- 自动模型管理：模型打包在应用中，自动提取到私有目录
- 类型安全：通过 Codegen 生成类型安全的 TypeScript 接口

---

## 双端实现

### Android 实现

Android 端通过 JNI 桥接实现 C++ 调用：

```
React Native (JS/TS)
        ↓ JSI
TurboModule (C++)
        ↓ JNI
ModelExtractor (Kotlin)
        ↓
Android Assets (RFB-320.mnn)
```

**关键文件：**

| 文件 | 说明 |
|------|------|
| [android/app/src/main/java/com/anonymous/test_mnn/ModelExtractor.kt](android/app/src/main/java/com/anonymous/test_mnn/ModelExtractor.kt) | 模型路径管理，从 assets 提取模型到缓存目录 |
| [android/app/src/main/cpp/CMakeLists.txt](android/app/src/main/cpp/CMakeLists.txt) | C++ 编译配置，链接 MNN 和 OpenCV |
| android/app/src/main/assets/RFB-320.mnn | 人脸检测模型文件 |
| android/app/src/main/jniLibs/arm64-v8a/libMNN.so | MNN 动态库 |

**模型加载流程：**

1. 应用启动时，`ModelExtractor.getModelPath()` 从 assets 读取 `RFB-320.mnn`
2. 模型被提取到应用的 cache 目录
3. 通过 JNI 调用 `nativeSetModelPath()` 将路径传递给 C++ 层
4. C++ 层使用该路径初始化 MNN 解释器

### iOS 实现

iOS 端通过 Objective-C++ 桥接实现 C++ 调用：

```
React Native (JS/TS)
        ↓ JSI
TurboModule (C++)
        ↓ Objective-C++
iOSModelLoader (Objective-C++)
        ↓
iOS Bundle Resources (RFB-320.mnn)
```

**关键文件：**

| 文件 | 说明 |
|------|------|
| [ios/iOSModelLoader.mm](ios/iOSModelLoader.mm) | 模型路径管理，提供 C++ 接口访问模型路径 |
| [ios/testmnn/AppDelegate.swift](ios/testmnn/AppDelegate.swift) | 应用启动时初始化模型路径 |
| ios/RFB-320.mnn | 人脸检测模型文件（Copy Bundle Resources） |
| ios/libs/libMNN.a | MNN 静态库 |

**模型加载流程：**

1. 应用启动时，`AppDelegate.swift` 从 Bundle 中查找 `RFB-320.mnn`
2. 调用 `iOSModelLoader.setModelPath()` 保存模型路径
3. C++ 层通过 `getIOSModelPath()` 函数获取路径
4. 初始化 MNN 解释器进行推理

---

## 共享 C++ 实现

双端共用以下 C++ 代码：

| 文件 | 说明 |
|------|------|
| [shared/NativeFaceDetector.h](shared/NativeFaceDetector.h) | 人脸检测器头文件 |
| [shared/NativeFaceDetector.cpp](shared/NativeFaceDetector.cpp) | 人脸检测器实现（MNN + UltraFace） |
| [shared/NativeSampleModule.h](shared/NativeSampleModule.h) | TurboModule 头文件 |
| [shared/NativeSampleModule.cpp](shared/NativeSampleModule.cpp) | TurboModule 实现 |

**NativeFaceDetector 功能：**

- 模型初始化：加载 MNN 模型，配置输入输出张量
- 图像预处理：BGR → RGB 转换、归一化、缩放到 320x240
- Anchor 生成：基于 UltraFace 的 anchor 策略
- 人脸检测：运行推理，解析输出
- NMS 后处理：去除重复检测框

---

## 使用方法

### 1. 初始化人脸检测器

```typescript
import NativeSampleModule from '@/specs/NativeSampleModule';

// 模型会自动从 assets 加载
const result = await NativeSampleModule.initFaceDetector();
// 返回: {"status":"success","message":"Detector initialized"}
```

### 2. 检测人脸

```typescript
const imagePath = '/path/to/face.jpg';
const result = await NativeSampleModule.detectFace(imagePath);
// 返回: {"faces":[{"x":100,"y":150,"width":200,"height":250,"score":0.98}]}
```

### Demo 界面

运行应用后，在 Native Demo 页面可以：

1. **字符串反转测试** - 验证 TurboModule 基础功能
2. **数字相加测试** - 验证 C++ 数值计算
3. **人脸检测** - 初始化检测器并检测图片中的人脸

---

## 环境要求

| 工具 | 版本要求 |
|------|---------|
| Node.js | ≥18 |
| React Native | 0.81+ |
| Xcode | ≥15 |
| Android Studio | ≥2023 |

## 第三方依赖

- **MNN** - [GitHub](https://github.com/alibaba/MNN)
- **OpenCV Mobile** - [ Releases](https://github.com/nihui/opencv-mobile/releases)
- **UltraFace** - [GitHub](https://github.com/Linzaer/Ultra-Light-Fast-Generic-Face-Detector-1MB)

## 详细文档

- [TurboModule 实现指南](docs/TURBO_MODULE_GUIDE.md)
- [人脸检测完整实现指南](docs/TURBOMODULE_FACE_DETECTION_GUIDE.md)

## 项目结构

```
mnn_test/
├── android/                      # Android 原生代码
│   └── app/
│       ├── src/main/
│       │   ├── assets/
│       │   │   └── RFB-320.mnn  # 人脸检测模型
│       │   ├── java/.../ModelExtractor.kt
│       │   ├── cpp/              # JNI/C++ 代码
│       │   └── jniLibs/          # libMNN.so
├── ios/                          # iOS 原生代码
│   ├── RFB-320.mnn              # 人脸检测模型
│   ├── iOSModelLoader.mm        # 模型路径管理
│   └── libs/                    # libMNN.a, opencv2.framework
├── shared/                       # 跨平台 C++ 代码
│   ├── NativeFaceDetector.cpp/h
│   └── NativeSampleModule.cpp/h
├── specs/                        # TurboModule Spec
│   └── NativeSampleModule.ts
├── app/                          # React Native 代码
│   └── (tabs)/native-demo.tsx   # Demo 页面
└── docs/                         # 文档
```
