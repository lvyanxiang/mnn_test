#include "NativeFaceDetector.h"

// 平台特定的头文件和日志宏
#ifdef __ANDROID__
  #include <android/log.h>
  #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
  #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#else
  #include <cstdio>
  #define LOGI(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
  #define LOGE(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#endif

#include <algorithm>
#include <cmath>
#include <iostream>

#define TAG "NativeFaceDetector"

// 使用 OpenCV 的 MAX/MIN 宏，不再重复定义
#define Clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

namespace facebook::react {

NativeFaceDetector::NativeFaceDetector()
    : initialized_(false)
    , session_(nullptr)
    , inputTensor_(nullptr) {
}

NativeFaceDetector::~NativeFaceDetector() {
    if (interpreter_) {
        interpreter_->releaseModel();
        interpreter_->releaseSession(session_);
    }
}

int NativeFaceDetector::init(const std::string& modelPath) {
    LOGI("Start init NativeFaceDetector");
    LOGI("Model path: %s", modelPath.c_str());

    // 创建 MNN 解释器
    interpreter_ = std::shared_ptr<MNN::Interpreter>(
        MNN::Interpreter::createFromFile(modelPath.c_str()));

    if (nullptr == interpreter_) {
        LOGE("Failed to load model");
        return 10000;
    }

    // 配置会话
    MNN::ScheduleConfig scheduleConfig;
    scheduleConfig.type = MNN_FORWARD_CPU;
    scheduleConfig.numThread = 2;

    MNN::BackendConfig backendConfig;
    backendConfig.memory = MNN::BackendConfig::Memory_Normal;
    backendConfig.power = MNN::BackendConfig::Power_Normal;
    backendConfig.precision = MNN::BackendConfig::Precision_Normal;
    scheduleConfig.backendConfig = &backendConfig;

    // 创建会话
    session_ = interpreter_->createSession(scheduleConfig);

    // 配置输入张量
    inputTensor_ = interpreter_->getSessionInput(session_, nullptr);
    interpreter_->resizeTensor(inputTensor_, {1, 3, inputSizeHeight_, inputSizeWidth_});
    interpreter_->resizeSession(session_);

    // 配置图像预处理
    MNN::CV::Matrix trans;
    trans.setScale(1.0f, 1.0f);

    MNN::CV::ImageProcess::Config imgConfig;
    imgConfig.filterType = MNN::CV::BICUBIC;
    memcpy(imgConfig.mean, meanVals_, sizeof(meanVals_));
    memcpy(imgConfig.normal, normVals_, sizeof(normVals_));
    imgConfig.sourceFormat = MNN::CV::BGR;
    imgConfig.destFormat = MNN::CV::RGB;

    pretreat_ = std::shared_ptr<MNN::CV::ImageProcess>(
        MNN::CV::ImageProcess::create(imgConfig));
    pretreat_->setMatrix(trans);

    // 打印模型信息
    LOGI("=== Model Info ===");
    auto allInput = interpreter_->getSessionInputAll(session_);
    LOGI("Inputs: %zu", allInput.size());
    for (auto& iter : allInput) {
        LOGI("  Input name: %s", iter.first.c_str());
    }

    auto allOutput = interpreter_->getSessionOutputAll(session_);
    LOGI("Outputs: %zu", allOutput.size());
    for (auto& iter : allOutput) {
        LOGI("  Output name: %s", iter.first.c_str());
    }
    LOGI("=================");

    // 生成 anchors（来自 UltraFace）
    std::vector<std::vector<float>> minBoxes = {
        {10.0f, 16.0f, 24.0f},
        {32.0f, 48.0f},
        {64.0f, 96.0f},
        {128.0f, 192.0f, 256.0f}
    };
    std::vector<float> strides = {8.0f, 16.0f, 32.0f, 64.0f};
    generateAnchors(inputSizeWidth_, inputSizeHeight_, minBoxes, strides, &anchors_);

    initialized_ = true;
    LOGI("NativeFaceDetector initialized successfully");
    return 0;
}

int NativeFaceDetector::detect(const cv::Mat& img, std::vector<FaceInfo>* faces) {
    faces->clear();

    if (!initialized_) {
        LOGE("Model not initialized");
        return 10000;
    }

    if (img.empty()) {
        LOGE("Input image is empty");
        return 10001;
    }

    int width = img.cols;
    int height = img.rows;

    // 调整图像大小并预处理
    cv::Mat imgResized;
    cv::resize(img, imgResized, cv::Size(inputSizeWidth_, inputSizeHeight_));
    pretreat_->convert(imgResized.data, inputSizeWidth_, inputSizeHeight_,
                       imgResized.step[0], inputTensor_);

    // 运行推理
    interpreter_->runSession(session_);

    // 获取输出（参考实现使用硬编码的节点名称）
    auto tensorScore = interpreter_->getSessionOutput(session_, "scores");
    auto tensorBbox = interpreter_->getSessionOutput(session_, "boxes");

    // 如果找不到，尝试按顺序获取
    if (!tensorScore || !tensorBbox) {
        LOGI("Named outputs not found, trying by index");
        auto allOutput = interpreter_->getSessionOutputAll(session_);
        LOGI("Total outputs: %zu", allOutput.size());

        int outputIdx = 0;
        for (auto& iter : allOutput) {
            LOGI("Output[%d]: name=%s", outputIdx, iter.first.c_str());
            if (outputIdx == 0 && !tensorBbox) {
                tensorBbox = iter.second;
                LOGI("Using output '%s' as bbox (fallback)", iter.first.c_str());
            } else if (outputIdx == 1 && !tensorScore) {
                tensorScore = iter.second;
                LOGI("Using output '%s' as score (fallback)", iter.first.c_str());
            }
            outputIdx++;
        }
    } else {
        LOGI("Using named outputs: scores=%p, boxes=%p", tensorScore, tensorBbox);
    }

    if (!tensorScore || !tensorBbox) {
        LOGE("Failed to get output tensors: score=%p, bbox=%p", tensorScore, tensorBbox);
        return 10002;
    }

    MNN::Tensor hostScore(tensorScore, tensorScore->getDimensionType());
    MNN::Tensor hostBbox(tensorBbox, tensorBbox->getDimensionType());
    tensorScore->copyToHostTensor(&hostScore);
    tensorBbox->copyToHostTensor(&hostBbox);

    // 打印前几个score值（调试用）
    const float* scoreData = hostScore.host<float>();
    LOGI("First 5 score pairs (bg, face): %.3f,%.3f | %.3f,%.3f | %.3f,%.3f | %.3f,%.3f | %.3f,%.3f",
         scoreData[0], scoreData[1], scoreData[2], scoreData[3], scoreData[4], scoreData[5],
         scoreData[6], scoreData[7], scoreData[8], scoreData[9]);

    // 解析结果
    int numAnchors = static_cast<int>(anchors_.size());
    std::vector<FaceInfo> facesTmp;

    const float centerVariance = 0.1f;
    const float sizeVariance = 0.2f;

    for (int i = 0; i < numAnchors; ++i) {
        float score = hostScore.host<float>()[2 * i + 1];
        if (score <= scoreThreshold_) continue;

        FaceInfo faceInfo;

        // 计算边界框
        float centerX = hostBbox.host<float>()[4 * i] * centerVariance * anchors_[i][2] + anchors_[i][0];
        float centerY = hostBbox.host<float>()[4 * i + 1] * centerVariance * anchors_[i][3] + anchors_[i][1];
        float centerW = exp(hostBbox.host<float>()[4 * i + 2] * sizeVariance) * anchors_[i][2];
        float centerH = exp(hostBbox.host<float>()[4 * i + 3] * sizeVariance) * anchors_[i][3];

        cv::Rect face;
        face.x = static_cast<int>(Clip(centerX - centerW / 2.0f, 1.0f) * width);
        face.y = static_cast<int>(Clip(centerY - centerH / 2.0f, 1.0f) * height);
        face.width = static_cast<int>(Clip(centerW, 1.0f) * width);
        face.height = static_cast<int>(Clip(centerH, 1.0f) * height);

        // 转换为正方形
        int maxSide = MAX(face.width, face.height);
        faceInfo.x = face.x + 0.5f * face.width - 0.5f * maxSide;
        faceInfo.y = face.y + 0.5f * face.height - 0.5f * maxSide;
        faceInfo.width = maxSide;
        faceInfo.height = maxSide;
        faceInfo.x = Clip(faceInfo.x, width - maxSide);
        faceInfo.y = Clip(faceInfo.y, height - maxSide);

        faceInfo.score = Clip(score, 1.0f);
        facesTmp.push_back(faceInfo);
    }

    // NMS 去重
    nms(facesTmp, faces, iouThreshold_);

    LOGI("Detected %zu faces", faces->size());
    return 0;
}

// 生成 anchors
int NativeFaceDetector::generateAnchors(
    const int& width, const int& height,
    const std::vector<std::vector<float>>& minBoxes,
    const std::vector<float>& strides,
    std::vector<std::vector<float>>* anchors) {

    anchors->clear();
    int numStrides = static_cast<int>(strides.size());

    for (int i = 0; i < numStrides; ++i) {
        auto stride = strides[i];

        int numX = ceil(width / stride);
        int numY = ceil(height / stride);

        for (int y = 0; y < numY; ++y) {
            for (int x = 0; x < numX; ++x) {
                float centerX = (x + 0.5f) * stride / width;
                float centerY = (y + 0.5f) * stride / height;

                for (auto minBox : minBoxes[i]) {
                    float centerW = minBox / width;
                    float centerH = minBox / height;
                    anchors->push_back({
                        Clip(centerX, 1.0f),
                        Clip(centerY, 1.0f),
                        Clip(centerW, 1.0f),
                        Clip(centerH, 1.0f)
                    });
                }
            }
        }
    }

    LOGI("Generated %zu anchors", anchors->size());
    return 0;
}

// NMS 去重
void NativeFaceDetector::nms(
    const std::vector<FaceInfo>& inputs,
    std::vector<FaceInfo>* result,
    const float& threshold) {

    result->clear();
    if (inputs.size() == 0) return;

    std::vector<FaceInfo> inputsTmp = inputs;

    // 按置信度排序
    std::sort(inputsTmp.begin(), inputsTmp.end(),
        [](const FaceInfo& a, const FaceInfo& b) {
            return a.score > b.score;
        });

    std::vector<int> indexes(inputsTmp.size());
    for (size_t i = 0; i < indexes.size(); i++) {
        indexes[i] = i;
    }

    while (indexes.size() > 0) {
        int indexGood = indexes[0];
        std::vector<int> indexesTmp = indexes;
        indexes.clear();

        std::vector<int> indexesNms;
        indexesNms.push_back(indexGood);

        for (size_t i = 1; i < indexesTmp.size(); ++i) {
            int indexTmp = indexesTmp[i];

            // 计算 IoU
            float iou = 0.0f;
            float x1 = std::max(inputsTmp[indexGood].x, inputsTmp[indexTmp].x);
            float y1 = std::max(inputsTmp[indexGood].y, inputsTmp[indexTmp].y);
            float x2 = std::min(inputsTmp[indexGood].x + inputsTmp[indexGood].width,
                              inputsTmp[indexTmp].x + inputsTmp[indexTmp].width);
            float y2 = std::min(inputsTmp[indexGood].y + inputsTmp[indexGood].height,
                              inputsTmp[indexTmp].y + inputsTmp[indexTmp].height);

            float w = std::max(0.0f, x2 - x1);
            float h = std::max(0.0f, y2 - y1);
            float interArea = w * h;
            float area1 = inputsTmp[indexGood].width * inputsTmp[indexGood].height;
            float area2 = inputsTmp[indexTmp].width * inputsTmp[indexTmp].height;
            iou = interArea / (area1 + area2 - interArea);

            if (iou <= threshold) {
                indexes.push_back(indexTmp);
            } else {
                indexesNms.push_back(indexTmp);
            }
        }

        result->push_back(inputsTmp[indexGood]);
    }
}

} // namespace facebook::react
