#pragma once

#include <AppSpecsJSI.h>
#include <jsi/jsi.h>

namespace facebook::react {

class NativeSampleModule : public NativeSampleModuleCxxSpec<NativeSampleModule> {
public:
  NativeSampleModule(std::shared_ptr<CallInvoker> jsInvoker);

  jsi::String reverseString(jsi::Runtime& rt, jsi::String input);
};

} // namespace facebook::react
