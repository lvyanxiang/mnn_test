#include "NativeSampleModule.h"

namespace facebook::react {

NativeSampleModule::NativeSampleModule(std::shared_ptr<CallInvoker> jsInvoker)
    : NativeSampleModuleCxxSpec(std::move(jsInvoker)) {}

jsi::String NativeSampleModule::reverseString(jsi::Runtime& rt, jsi::String input) {
  // Convert jsi::String to std::string
  std::string inputStr = input.utf8(rt);

  // Reverse the string
  std::string reversed(inputStr.rbegin(), inputStr.rend());

  // Convert back to jsi::String
  return jsi::String::createFromUtf8(rt, reversed);
}

} // namespace facebook::react
