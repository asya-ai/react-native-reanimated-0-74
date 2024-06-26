#include "RNRuntimeDecorator.h"
#include "ReanimatedJSIUtils.h"
#include "ReanimatedVersion.h"

namespace reanimated {

void RNRuntimeDecorator::decorate(
    jsi::Runtime &rnRuntime,
    const std::shared_ptr<NativeReanimatedModule> &nativeReanimatedModule,
    const bool isReducedMotion) {
  rnRuntime.global().setProperty(rnRuntime, "_WORKLET", false);

  jsi::Runtime &uiRuntime = nativeReanimatedModule->getUIRuntime();
  auto workletRuntimeValue =
      rnRuntime.global()
          .getPropertyAsObject(rnRuntime, "ArrayBuffer")
          .asFunction(rnRuntime)
          .callAsConstructor(rnRuntime, {static_cast<double>(sizeof(void *))});
  uintptr_t *workletRuntimeData = reinterpret_cast<uintptr_t *>(
      workletRuntimeValue.getObject(rnRuntime).getArrayBuffer(rnRuntime).data(
          rnRuntime));
  workletRuntimeData[0] = reinterpret_cast<uintptr_t>(&uiRuntime);
  rnRuntime.global().setProperty(
      rnRuntime, "_WORKLET_RUNTIME", workletRuntimeValue);

#ifdef RCT_NEW_ARCH_ENABLED
  constexpr auto isFabric = true;
#else
  constexpr auto isFabric = false;
#endif // RCT_NEW_ARCH_ENABLED
  rnRuntime.global().setProperty(rnRuntime, "_IS_FABRIC", isFabric);

  jsi_utils::installJsiFunction(
      rnRuntime,
      "_hasNativeState",
      [](jsi::Runtime &rt, const jsi::Value &value) {
#if REACT_NATIVE_MINOR_VERSION >= 71
        return value.asObject(rt).hasNativeState(rt);
#else
        return false;
#endif // REACT_NATIVE_MINOR_VERSION >= 71
      });

#ifndef NDEBUG
  checkJSVersion(rnRuntime, nativeReanimatedModule->getJSLogger());
#endif // NDEBUG
  injectReanimatedCppVersion(rnRuntime);

  rnRuntime.global().setProperty(
      rnRuntime, "_REANIMATED_IS_REDUCED_MOTION", isReducedMotion);

  rnRuntime.global().setProperty(
      rnRuntime,
      jsi::PropNameID::forAscii(rnRuntime, "__reanimatedModuleProxy"),
      jsi::Object::createFromHostObject(rnRuntime, nativeReanimatedModule));
}

} // namespace reanimated
