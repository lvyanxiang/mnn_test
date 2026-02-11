#include "ModelJni.h"
#include <jni.h>
#include <string>
#include <android/log.h>

#define TAG "ModelJni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

extern "C" {

// 全局变量保存模型路径
static std::string g_model_path;

/**
 * JNI 方法：设置模型路径（从 Java 调用）
 */
JNIEXPORT void JNICALL
Java_com_anonymous_test_1mnn_ModelExtractor_nativeSetModelPath(
    JNIEnv* env,
    jclass clazz,
    jstring modelPath) {
    LOGI("JNI: nativeSetModelPath called");

    if (modelPath == nullptr) {
        LOGE("JNI: modelPath parameter is null!");
        return;
    }

    const char* pathChars = env->GetStringUTFChars(modelPath, nullptr);
    g_model_path = std::string(pathChars);
    env->ReleaseStringUTFChars(modelPath, pathChars);

    LOGI("JNI: Model path set to: %s (length: %zu)", g_model_path.c_str(), g_model_path.length());
}

/**
 * JNI 方法：获取模型路径（从 C++ 调用）
 */
const char* getModelPath() {
    LOGI("JNI: getModelPath() called, g_model_path.empty()=%s", g_model_path.empty() ? "true" : "false");
    if (g_model_path.empty()) {
        LOGE("JNI: g_model_path is empty, returning nullptr");
        return nullptr;
    }
    LOGI("JNI: Returning model path: %s", g_model_path.c_str());
    return g_model_path.c_str();
}

} // extern "C"
