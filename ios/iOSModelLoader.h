#pragma once

#import <Foundation/Foundation.h>

@interface iOSModelLoader : NSObject

+ (void)setModelPath:(NSString *)modelPath;
+ (NSString *)getModelPath;

@end

// C++ 接口（供 C++ 代码调用）
#ifdef __cplusplus
extern "C" {
    // C++ 代码调用此函数获取模型路径
    const char* getIOSModelPath();
}
#endif
