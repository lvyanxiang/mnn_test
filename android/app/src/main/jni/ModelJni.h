#pragma once

#include <cstdlib>

/**
 * 获取模型文件路径
 * @return 模型文件的完整路径，如果未设置则返回 nullptr
 */
extern "C" const char* getModelPath();
