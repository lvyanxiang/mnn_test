# MNN Face Detection Demo

> Cross-platform face detection solution using React Native TurboModule + MNN + UltraFace

A cross-platform face recognition application built with React Native TurboModule (New Architecture), powered by MNN inference engine and UltraFace model.

---

**Keywords:**

`React Native` `TurboModule` `New Architecture` `JSI` `MNN` `Face Detection` `Face Recognition` `UltraFace` `OpenCV` `C++` `Cross-platform` `iOS` `Android` `Deep Learning` `Inference` `JNI` `Objective-C++` `Expo` `Computer Vision` `Mobile AI` `On-device AI`

---

## Project Overview

This project demonstrates how to implement cross-platform face detection in React Native using **TurboModule (New Architecture)**.

### Core Technologies

- **MNN** - Lightweight deep learning inference engine by Alibaba
- **OpenCV** - Computer vision and image processing library
- **UltraFace (RFB-320)** - Ultra-lightweight face detection model (~1MB)
- **TurboModule** - React Native's new architecture with zero-copy JSI calls

### Architecture Features

- **Zero-copy calls**: Direct C++ invocation via JSI, no Bridge serialization overhead
- **Code reuse**: Single C++ implementation shared between iOS and Android
- **Auto model management**: Models bundled in-app, automatically extracted to private directories
- **Type safety**: Type-safe TypeScript interfaces generated via Codegen

### Why Self-Implementation? 🚀

**Advantages over third-party face detection SDKs (Google ML Kit, Apple Face Detection, Azure Face API, etc.):**

| Aspect | Self-Implementation | Third-Party SDKs |
|--------|-------------------|-------------------|
| **Privacy** | ✅ 100% on-device, no data leaves the device | ❌ May require cloud API calls |
| **Offline Capability** | ✅ Works completely offline | ⚠️ Some require network |
| **Cost** | ✅ Free, no API call fees | 💰 Often charge per API call |
| **Bundle Size** | ✅ ~2MB (MNN + model) | 📦 Can add 10-50MB+ |
| **Customization** | ✅ Full control over threshold, NMS, anchors | ❌ Black-box, limited tuning |
| **Dependencies** | ✅ Minimal, no vendor lock-in | 🔒 Vendor-specific dependencies |
| **Performance** | ⚡ Optimized for your specific use case | ⚠️ General-purpose optimizations |
| **Latency** | ⚡ No network round-trip | 🐌 Network delays if cloud-based |
| **Learning Value** | 🎓 Deep understanding of ML pipeline | ⚠️ Abstracted away |

**Key Benefits:**

1. **Privacy First** - All processing happens on-device, images never leave the user's phone
2. **No Vendor Lock-in** - Switch models or inference engines anytime (MNN → NCNN → TFLite)
3. **Cost Effective** - Zero recurring costs, no per-call API fees
4. **Lightweight** - UltraFace model is only ~1MB vs 20-50MB for many SDKs
5. **Full Control** - Adjust detection threshold, NMS parameters, anchor strategies for your use case
6. **Offline-First** - Works in airplane mode, remote areas, or privacy-sensitive environments
7. **Cross-Platform Consistency** - Same model behavior across iOS and Android
8. **Educational** - Learn the complete ML pipeline: preprocessing → inference → post-processing

**Trade-offs to Consider:**

- ⚠️ Requires ML/C++ knowledge to maintain and optimize
- ⚠️ Need to handle model updates yourself
- ⚠️ Less polished than production SDKs initially (but you control the roadmap!)

---

## Performance Comparison

| Metric | This Implementation | Google ML Kit | Apple Vision | Azure Face API |
|---------|-------------------|----------------|--------------|----------------|
| **Bundle Size** | ~2 MB | ~8 MB | System framework | N/A (cloud) |
| **Detection Speed** | ~50ms on CPU | ~100ms | ~80ms | ~200ms+ |
| **API Call Cost** | $0 | $0 | $0 | $$/1000 calls |
| **Privacy** | ✅ On-device | ✅ On-device | ✅ On-device | ❌ Cloud |
| **Offline** | ✅ Yes | ✅ Yes | ✅ Yes | ❌ No |
| **Customizable** | ✅ Full source | ❌ Binary | ❌ Binary | ❌ API only |

*Note: Benchmarks vary by device, image size, and network conditions*

## Decision Guide: When to Use Which?

### ✅ **Choose Self-Implementation (This Project)** when you need:

- Privacy guarantees with no cloud communication
- Full control over detection behavior
- Zero recurring costs
- Minimal app size impact
- Cross-platform consistency
- Custom detection scenarios (side profiles, low light, etc.)
- Learning ML deployment fundamentals

### ❌ **Consider Third-Party SDKs** when you need:

- Liveness detection (anti-spoofing)
- Face recognition (identification), not just detection
- Landmark detection (eyes, nose, mouth positions)
- Emotion/recognition attributes
- Rapid prototyping without ML expertise
- Enterprise support and SLAs

### 🔄 **Hybrid Approach**

You can also start here and migrate:
1. **MVP**: Use this implementation for fast, cheap validation
2. **Scale**: Migrate to cloud API only for complex features
3. **Optimize**: Fine-tune model for your specific user demographics

---

### Feature Tags

🎯 **Face Detection** | 📱 **Cross-platform** | ⚡ **High Performance** | 🔧 **TurboModule** | 🧠 **MNN Inference** | 📸 **Real-time Detection** | 🎓 **Learning Example**

---

## Search Relevance

This project is ideal for developers searching for:

- ✅ React Native TurboModule tutorial / examples / best practices
- ✅ React Native New Architecture / JSI implementation
- ✅ MNN mobile deployment / MNN React Native integration
- ✅ Face detection / ultra-lightweight face detection
- ✅ UltraFace MNN version / mobile face detection
- ✅ React Native C++ cross-platform development
- ✅ JNI / Objective-C++ bridging examples
- ✅ Expo native module integration
- ✅ Mobile deep learning inference / on-device AI
- ✅ React Native computer vision

---

---

## Platform Implementation

### Android Implementation

Android uses JNI bridging for C++ invocation:

```
React Native (JS/TS)
        ↓ JSI
TurboModule (C++)
        ↓ JNI
ModelExtractor (Kotlin)
        ↓
Android Assets (RFB-320.mnn)
```

**Key Files:**

| File | Description |
|------|-------------|
| [android/app/src/main/java/com/anonymous/test_mnn/ModelExtractor.kt](android/app/src/main/java/com/anonymous/test_mnn/ModelExtractor.kt) | Model path management, extracts model from assets to cache directory |
| [android/app/src/main/cpp/CMakeLists.txt](android/app/src/main/cpp/CMakeLists.txt) | C++ build configuration, links MNN and OpenCV |
| android/app/src/main/assets/RFB-320.mnn | Face detection model file |
| android/app/src/main/jniLibs/arm64-v8a/libMNN.so | MNN shared library |

**Model Loading Flow:**

1. On app startup, `ModelExtractor.getModelPath()` reads `RFB-320.mnn` from assets
2. Model is extracted to app's cache directory
3. JNI call `nativeSetModelPath()` passes the path to C++ layer
4. C++ layer initializes MNN interpreter with the model path

### iOS Implementation

iOS uses Objective-C++ bridging for C++ invocation:

```
React Native (JS/TS)
        ↓ JSI
TurboModule (C++)
        ↓ Objective-C++
iOSModelLoader (Objective-C++)
        ↓
iOS Bundle Resources (RFB-320.mnn)
```

**Key Files:**

| File | Description |
|------|-------------|
| [ios/iOSModelLoader.mm](ios/iOSModelLoader.mm) | Model path management, provides C++ interface to access model path |
| [ios/testmnn/AppDelegate.swift](ios/testmnn/AppDelegate.swift) | Initializes model path on app startup |
| ios/RFB-320.mnn | Face detection model file (Copy Bundle Resources) |
| ios/libs/libMNN.a | MNN static library |

**Model Loading Flow:**

1. On app startup, `AppDelegate.swift` locates `RFB-320.mnn` from Bundle
2. Calls `iOSModelLoader.setModelPath()` to save the model path
3. C++ layer retrieves path via `getIOSModelPath()` function
4. Initializes MNN interpreter for inference

---

## Shared C++ Implementation

Both platforms share the following C++ code:

| File | Description |
|------|-------------|
| [shared/NativeFaceDetector.h](shared/NativeFaceDetector.h) | Face detector header file |
| [shared/NativeFaceDetector.cpp](shared/NativeFaceDetector.cpp) | Face detector implementation (MNN + UltraFace) |
| [shared/NativeSampleModule.h](shared/NativeSampleModule.h) | TurboModule header file |
| [shared/NativeSampleModule.cpp](shared/NativeSampleModule.cpp) | TurboModule implementation |

**NativeFaceDetector Features:**

- Model initialization: Loads MNN model, configures input/output tensors
- Image preprocessing: BGR → RGB conversion, normalization, resize to 320x240
- Anchor generation: Based on UltraFace anchor strategy
- Face detection: Runs inference, parses output
- NMS post-processing: Removes duplicate detection boxes

---

## Ideal Use Cases

**Perfect for applications requiring:**

- 🔒 **Privacy-focused apps** - Photo editors, gallery apps, health apps
- 📱 **Offline-first apps** - Field work, travel, remote areas
- 💰 **Cost-sensitive apps** - Startups, MVPs, high-volume usage
- 🎯 **Niche requirements** - Custom face sizes, specific lighting conditions
- 📚 **Educational projects** - Learn ML deployment on mobile
- 🏢 **Enterprise apps** - No data leaves company devices
- 🎮 **Real-time apps** - Camera filters, AR effects, games

**Real-world examples:**
- Employee attendance system with privacy guarantees
- Photo album auto-categorization
- Camera face filters/effects
- Access control with local verification
- Kids' mode detection in parental controls
- Automated selfie capture for profiles

---

### 1. Initialize Face Detector

```typescript
import NativeSampleModule from '@/specs/NativeSampleModule';

// Model automatically loads from assets
const result = await NativeSampleModule.initFaceDetector();
// Returns: {"status":"success","message":"Detector initialized"}
```

### 2. Detect Faces

```typescript
const imagePath = '/path/to/face.jpg';
const result = await NativeSampleModule.detectFace(imagePath);
// Returns: {"faces":[{"x":100,"y":150,"width":200,"height":250,"score":0.98}]}
```

### Demo Interface

After running the app, navigate to the Native Demo page to:

1. **String Reverse Test** - Verify basic TurboModule functionality
2. **Number Addition Test** - Verify C++ numerical computation
3. **Face Detection** - Initialize detector and detect faces in images

---

## Requirements

| Tool | Version |
|------|---------|
| Node.js | ≥18 |
| React Native | 0.81+ |
| Xcode | ≥15 |
| Android Studio | ≥2023 |

## Third-party Dependencies

- **MNN** - [GitHub](https://github.com/alibaba/MNN)
- **OpenCV Mobile** - [Releases](https://github.com/nihui/opencv-mobile/releases)
- **UltraFace** - [GitHub](https://github.com/Linzaer/Ultra-Light-Fast-Generic-Face-Detector-1MB)

## Documentation

- [TurboModule Implementation Guide](docs/TURBO_MODULE_GUIDE.md)
- [Face Detection Complete Guide](docs/TURBOMODULE_FACE_DETECTION_GUIDE.md)

## Project Structure

```
mnn_test/
├── android/                      # Android native code
│   └── app/
│       ├── src/main/
│       │   ├── assets/
│       │   │   └── RFB-320.mnn  # Face detection model
│       │   ├── java/.../ModelExtractor.kt
│       │   ├── cpp/              # JNI/C++ code
│       │   └── jniLibs/          # libMNN.so
├── ios/                          # iOS native code
│   ├── RFB-320.mnn              # Face detection model
│   ├── iOSModelLoader.mm        # Model path management
│   └── libs/                    # libMNN.a, opencv2.framework
├── shared/                       # Cross-platform C++ code
│   ├── NativeFaceDetector.cpp/h
│   └── NativeSampleModule.cpp/h
├── specs/                        # TurboModule Spec
│   └── NativeSampleModule.ts
├── app/                          # React Native code
│   └── (tabs)/native-demo.tsx   # Demo page
└── docs/                         # Documentation
```

---

## Tech Stack Tags

```
react-native turbonmodule jsi mnn face-detection ultraface
opencv cpp android ios kotlin swift objective-cpp
jni deep-learning inference mobile computer-vision expo
```

## Related Topics

- 🔥 [React Native New Architecture Docs](https://reactnative.dev/docs/the-new-architecture/landing-page)
- 🔥 [MNN Official Documentation](https://mnn.readthedocs.io/)
- 🔥 [UltraFace Paper & Model](https://github.com/Linzaer/Ultra-Light-Fast-Generic-Face-Detector-1MB)

---

**Search Suggestions:** This project may help if you're searching for:

- "React Native TurboModule tutorial"
- "React Native C++ module development"
- "MNN face detection mobile"
- "React Native face recognition implementation"
- "UltraFace React Native integration"
- "Cross-platform deep learning inference"
- "React Native JSI C++ calls"
- "Expo native module face detection"

---

## License

MIT
