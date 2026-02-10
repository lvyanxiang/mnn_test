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

    // 模型参数（来自 UltraFace）
    const int inputSizeWidth_ = 320;
    const int inputSizeHeight_ = 240;
    const float meanVals_[3] = {127.0f, 127.0f, 127.0f};
    const float normVals_[3] = {1.0f / 128.0f, 1.0f / 128.0f, 1.0f / 128.0f};
    const float scoreThreshold_ = 0.95f;  // 提高阈值减少误检
    const float iouThreshold_ = 0.3f;

    // 生成 anchors
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
