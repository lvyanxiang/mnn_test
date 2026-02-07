#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_anonymous_test_1mnn_MnnTestModule_nativeTestCall(
    JNIEnv *env,
    jobject /* this */) {
    std::string message = "Native call successful!";
    return env->NewStringUTF(message.c_str());
}
