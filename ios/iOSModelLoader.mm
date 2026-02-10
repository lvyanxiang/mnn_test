#import "iOSModelLoader.h"
#import <Foundation/Foundation.h>

// 全局变量保存模型路径（供 C++ 访问）
static NSString* g_model_path = nil;

// 前向声明 C++ 函数
#ifdef __cplusplus
extern "C" {
    void setIOSModelPath(const char* path);
}
#endif

// Objective-C++ 实现的模型路径设置函数
@implementation iOSModelLoader

+ (void)setModelPath:(NSString *)modelPath {
    if (modelPath) {
        g_model_path = [modelPath copy];
        NSLog(@"[iOSModelLoader] Model path set: %@", g_model_path);

        // 调用 C++ 函数设置模型路径
        setIOSModelPath([g_model_path UTF8String]);
    } else {
        NSLog(@"[iOSModelLoader] ERROR: modelPath is nil!");
    }
}

+ (NSString *)getModelPath {
    return g_model_path;
}

@end

// C++ 接口函数（供 C++ 代码调用）
#ifdef __cplusplus
extern "C" {
    const char* getIOSModelPath() {
        if (g_model_path) {
            return [g_model_path UTF8String];
        }
        return nullptr;
    }

    void setIOSModelPath(const char* path) {
        if (path) {
            g_model_path = [NSString stringWithUTF8String:path];
        }
    }
}
#endif
