#include "NativeSampleModule.h"
#include <fstream>
#include <string>
#include <vector>

// 平台特定的头文件和日志宏
#ifdef __ANDROID__
  #include "ModelJni.h"
  #include <android/log.h>
  #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
  #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#else
  // iOS - 使用纯 C 前向声明，不包含 Objective-C++ 头文件
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
  // 创建人脸检测器实例
  faceDetector_ = std::make_unique<NativeFaceDetector>();
#ifdef __ANDROID__
  LOGI("NativeSampleModule created (Android)");
#else
  LOGI("NativeSampleModule created (iOS)");
#endif
}

NativeSampleModule::~NativeSampleModule() {
#ifdef __ANDROID__
  LOGI("NativeSampleModule destroyed (Android)");
#else
  LOGI("NativeSampleModule destroyed (iOS)");
#endif
}

jsi::String NativeSampleModule::reverseString(jsi::Runtime& rt, jsi::String input) {
  // Convert jsi::String to std::string
  std::string inputStr = input.utf8(rt);

  // Reverse the string
  std::string reversed(inputStr.rbegin(), inputStr.rend());

  // Convert back to jsi::String
  return jsi::String::createFromUtf8(rt, reversed);
}

double NativeSampleModule::addNumbers(jsi::Runtime& rt, double a, double b) {
  return a + b;
}

#ifdef __ANDROID__
jsi::String NativeSampleModule::initFaceDetector(jsi::Runtime& rt) {
  LOGI("initFaceDetector called (Android)");

  // 从 JNI 获取模型路径
  const char* modelPath = getModelPath();
  if (modelPath == nullptr) {
    LOGE("Model path not set. Make sure ModelExtractor.getModelPath() was called.");
    std::string error = R"({"error":"Model path not available. Please restart the app."})";
    return jsi::String::createFromUtf8(rt, error);
  }

  std::string pathStr(modelPath);
  LOGI("Model path: %s", pathStr.c_str());

  // 检查文件是否存在
  std::ifstream file(pathStr);
  if (!file.good()) {
    LOGE("Model file does not exist: %s", pathStr.c_str());
    std::string error = "{\"error\":\"Model file not found: " + pathStr + "\"}";
    return jsi::String::createFromUtf8(rt, error);
  }
  file.close();

  // 初始化检测器
  int ret = faceDetector_->init(pathStr);
  if (ret != 0) {
    LOGE("Failed to initialize face detector, error code: %d", ret);
    std::string error = "{\"error\":\"Failed to initialize detector\",\"code\":" + std::to_string(ret) + "}";
    return jsi::String::createFromUtf8(rt, error);
  }

  detectorInitialized_ = true;
  LOGI("Face detector initialized successfully (Android)");

  std::string result = "{\"status\":\"success\",\"message\":\"Detector initialized (Android)\"}";
  return jsi::String::createFromUtf8(rt, result);
}

jsi::String NativeSampleModule::detectFace(jsi::Runtime& rt, jsi::String imagePath) {
  std::string pathStr = imagePath.utf8(rt);

  LOGI("detectFace called (Android) with path: %s", pathStr.c_str());

  // 检查检测器是否已初始化
  if (!detectorInitialized_) {
    LOGE("Face detector not initialized");
    std::string error = R"({"error":"Detector not initialized. Call initFaceDetector first."})";
    return jsi::String::createFromUtf8(rt, error);
  }

  // 读取图像文件
  cv::Mat image = cv::imread(pathStr);
  if (image.empty()) {
    LOGE("Failed to read image: %s", pathStr.c_str());
    std::string error = R"({"error":"Failed to read image"})";
    return jsi::String::createFromUtf8(rt, error);
  }

  LOGI("Image loaded: %dx%d", image.cols, image.rows);

  // 调用检测器
  std::vector<FaceInfo> faces;
  int ret = faceDetector_->detect(image, &faces);
  if (ret != 0) {
    LOGE("Detection failed, error code: %d", ret);
    std::string error = "{\"error\":\"Detection failed\",\"code\":" + std::to_string(ret) + "}";
    return jsi::String::createFromUtf8(rt, error);
  }

  // 构建结果 JSON
  std::string json = "{\"faces\":[";
  for (size_t i = 0; i < faces.size(); i++) {
    const FaceInfo& face = faces[i];
    json += "{";
    json += "\"x\":" + std::to_string(static_cast<int>(face.x)) + ",";
    json += "\"y\":" + std::to_string(static_cast<int>(face.y)) + ",";
    json += "\"width\":" + std::to_string(static_cast<int>(face.width)) + ",";
    json += "\"height\":" + std::to_string(static_cast<int>(face.height)) + ",";
    json += "\"score\":" + std::to_string(face.score);
    json += "}";
    if (i < faces.size() - 1) {
      json += ",";
    }
  }
  json += "]}";

  LOGI("Detection result: %zu faces detected", faces.size());
  return jsi::String::createFromUtf8(rt, json);
}
#else
// iOS 实现
jsi::String NativeSampleModule::initFaceDetector(jsi::Runtime& rt) {
  LOGI("initFaceDetector called (iOS)");

  // 从 iOSModelLoader 获取模型路径
  const char* modelPath = getIOSModelPath();
  if (modelPath == nullptr) {
    LOGE("Model path not set. Make sure iOSModelLoader.setModelPath() was called in AppDelegate.");
    std::string error = R"({"error":"Model path not available. Please restart the app."})";
    return jsi::String::createFromUtf8(rt, error);
  }

  std::string pathStr(modelPath);
  LOGI("Model path: %s", pathStr.c_str());

  // 检查文件是否存在
  std::ifstream file(pathStr);
  if (!file.good()) {
    LOGE("Model file does not exist: %s", pathStr.c_str());
    std::string error = "{\"error\":\"Model file not found: " + pathStr + "\"}";
    return jsi::String::createFromUtf8(rt, error);
  }
  file.close();

  // 初始化检测器
  int ret = faceDetector_->init(pathStr);
  if (ret != 0) {
    LOGE("Failed to initialize face detector, error code: %d", ret);
    std::string error = "{\"error\":\"Failed to initialize detector\",\"code\":" + std::to_string(ret) + "}";
    return jsi::String::createFromUtf8(rt, error);
  }

  detectorInitialized_ = true;
  LOGI("Face detector initialized successfully (iOS)");

  std::string result = "{\"status\":\"success\",\"message\":\"Detector initialized (iOS)\"}";
  return jsi::String::createFromUtf8(rt, result);
}

jsi::String NativeSampleModule::detectFace(jsi::Runtime& rt, jsi::String imagePath) {
  std::string pathStr = imagePath.utf8(rt);

  LOGI("detectFace called (iOS) with path: %s", pathStr.c_str());

  // 检查检测器是否已初始化
  if (!detectorInitialized_) {
    LOGE("Face detector not initialized");
    std::string error = R"({"error":"Detector not initialized. Call initFaceDetector first."})";
    return jsi::String::createFromUtf8(rt, error);
  }

  // 读取图像文件
  cv::Mat image = cv::imread(pathStr);
  if (image.empty()) {
    LOGE("Failed to read image: %s", pathStr.c_str());
    std::string error = R"({"error":"Failed to read image"})";
    return jsi::String::createFromUtf8(rt, error);
  }

  LOGI("Image loaded: %dx%d", image.cols, image.rows);

  // 调用检测器
  std::vector<FaceInfo> faces;
  int ret = faceDetector_->detect(image, &faces);
  if (ret != 0) {
    LOGE("Detection failed, error code: %d", ret);
    std::string error = "{\"error\":\"Detection failed\",\"code\":" + std::to_string(ret) + "}";
    return jsi::String::createFromUtf8(rt, error);
  }

  // 构建结果 JSON
  std::string json = "{\"faces\":[";
  for (size_t i = 0; i < faces.size(); i++) {
    const FaceInfo& face = faces[i];
    json += "{";
    json += "\"x\":" + std::to_string(static_cast<int>(face.x)) + ",";
    json += "\"y\":" + std::to_string(static_cast<int>(face.y)) + ",";
    json += "\"width\":" + std::to_string(static_cast<int>(face.width)) + ",";
    json += "\"height\":" + std::to_string(static_cast<int>(face.height)) + ",";
    json += "\"score\":" + std::to_string(face.score);
    json += "}";
    if (i < faces.size() - 1) {
      json += ",";
    }
  }
  json += "]}";

  LOGI("Detection result (iOS): %zu faces detected", faces.size());
  return jsi::String::createFromUtf8(rt, json);
}
#endif

} // namespace facebook::react
