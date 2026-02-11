#pragma once

#include <AppSpecsJSI.h>
#include <jsi/jsi.h>
#include <memory>
#include "NativeFaceDetector.h"

namespace facebook::react {

class NativeSampleModule : public NativeSampleModuleCxxSpec<NativeSampleModule> {
public:
  NativeSampleModule(std::shared_ptr<CallInvoker> jsInvoker);
  ~NativeSampleModule();

  jsi::String reverseString(jsi::Runtime& rt, jsi::String input);
  double addNumbers(jsi::Runtime& rt, double a, double b);
  jsi::String initFaceDetector(jsi::Runtime& rt);  // 无需传参数，自动从加载的模型
  jsi::String detectFace(jsi::Runtime& rt, jsi::String imagePath);

private:
  std::unique_ptr<NativeFaceDetector> faceDetector_;
  bool detectorInitialized_;
};

} // namespace facebook::react
