# TurboModule 新增 C++ 方法指南

本文档说明如何在 React Native 新架构中为 TurboModule 添加新的 C++ 方法。

## 目录

- [步骤概述](#步骤概述)
- [详细步骤](#详细步骤)
- [数据类型映射](#数据类型映射)
- [Expo 项目注意事项](#expo-项目注意事项)
- [异步方法](#异步方法)
- [常见问题](#常见问题)

---

## 步骤概述

```
1. 修改 TypeScript 规范文件
       ↓
2. 运行 Codegen 生成绑定 (Expo 自动执行)
       ↓
3. 更新 C++ 头文件
       ↓
4. 实现 C++ 方法
       ↓
5. 重新编译
       ↓
6. JavaScript 调用
```

---

## 详细步骤

### 步骤 1: 修改 TypeScript 规范文件

**文件**: `specs/NativeSampleModule.ts`

添加新方法到 `Spec` 接口：

```typescript
import {TurboModule, TurboModuleRegistry} from 'react-native';

export interface Spec extends TurboModule {
  readonly reverseString: (input: string) => string;

  // 新增方法示例
  readonly addNumbers: (a: number, b: number) => number;
}

export default TurboModuleRegistry.getEnforcing<Spec>(
  'NativeSampleModule',
);
```

### 步骤 2: 重新生成 Codegen 绑定

**Expo 项目**（本项目的配置）：
```bash
# Codegen 会在构建时自动运行，无需手动执行
npm run android  # 或 npm run ios
```

**标准 React Native 项目**：
```bash
yarn react-native codegen
```

**自动生成的文件**（无需手动编辑）：

- `android/app/build/generated/source/codegen/java/.../NativeSampleModuleSpec.java`
- `android/app/build/generated/source/codegen/jni/react/renderer/components/AppSpecs/AppSpecsJSI.h`

> ⚠️ **注意**: 这些文件是自动生成的，任何手动修改都会在下次 codegen 时被覆盖。

### 步骤 3: 在 C++ 头文件中声明方法

**文件**: `shared/NativeSampleModule.h`

```cpp
#pragma once

#include <AppSpecsJSI.h>
#include <jsi/jsi.h>

namespace facebook::react {

class NativeSampleModule : public NativeSampleModuleCxxSpec<NativeSampleModule> {
public:
  NativeSampleModule(std::shared_ptr<CallInvoker> jsInvoker);

  jsi::String reverseString(jsi::Runtime& rt, jsi::String input);

  // 新增方法声明 - 注意返回类型是 double，不是 jsi::Value
  double addNumbers(jsi::Runtime& rt, double a, double b);
};

} // namespace facebook::react
```

> **关键**: 返回 `number` 类型的方法，在 C++ 中应使用 `double` 而不是 `jsi::Value`。Codegen 会根据 TypeScript 规范自动生成正确的类型。

### 步骤 4: 在 C++ 实现文件中实现方法

**文件**: `shared/NativeSampleModule.cpp`

```cpp
#include "NativeSampleModule.h"

namespace facebook::react {

NativeSampleModule::NativeSampleModule(std::shared_ptr<CallInvoker> jsInvoker)
    : NativeSampleModuleCxxSpec(std::move(jsInvoker)) {}

jsi::String NativeSampleModule::reverseString(jsi::Runtime& rt, jsi::String input) {
  std::string inputStr = input.utf8(rt);
  std::string reversed(inputStr.rbegin(), inputStr.rend());
  return jsi::String::createFromUtf8(rt, reversed);
}

// 新增方法实现
double NativeSampleModule::addNumbers(jsi::Runtime& rt, double a, double b) {
  return a + b;  // 直接返回 double，无需包装为 jsi::Value
}

} // namespace facebook::react
```

### 步骤 5: 重新编译

```bash
# Android
npm run android

# iOS
npm run ios
```

### 步骤 6: 在 JavaScript/TypeScript 中调用

```typescript
import NativeSampleModule from '@/specs/NativeSampleModule';

// 调用新方法
const sum = NativeSampleModule.addNumbers(10, 20);
console.log(sum); // 30
```

**完整的 UI 示例** (参考 `app/(tabs)/native-demo.tsx`):

```typescript
import React, { useState } from 'react';
import { Button, TextInput, View, Text } from 'react-native';
import NativeSampleModule from '@/specs/NativeSampleModule';

export default function DemoScreen() {
  const [numA, setNumA] = useState('10');
  const [numB, setNumB] = useState('20');
  const [result, setResult] = useState('');

  const handleAdd = () => {
    const a = parseFloat(numA) || 0;
    const b = parseFloat(numB) || 0;
    const sum = NativeSampleModule.addNumbers(a, b);
    setResult(String(sum));
  };

  return (
    <View>
      <TextInput
        value={numA}
        onChangeText={setNumA}
        keyboardType="numeric"
        placeholder="Number A"
      />
      <Text>+</Text>
      <TextInput
        value={numB}
        onChangeText={setNumB}
        keyboardType="numeric"
        placeholder="Number B"
      />
      <Button title="Add" onPress={handleAdd} />
      <Text>Result: {result}</Text>
    </View>
  );
}
```

---

## 数据类型映射

### TypeScript → C++ (JSI)

| TypeScript | C++ 参数类型 | C++ 返回类型 | 读取方式 | 创建方式 |
|-----------|------------|------------|---------|---------|
| `string` | `jsi::String` | `jsi::String` | `input.utf8(rt)` | `jsi::String::createFromUtf8(rt, str)` |
| `number` | `double` | `double` | 直接使用 | 直接返回数值 |
| `boolean` | `bool` | `bool` | 直接使用 | 直接返回布尔值 |
| `Object` | `jsi::Object` | `jsi::Object` | `obj.getProperty(rt, "key")` | `jsi::Object(rt)` |
| `Array` | `jsi::Array` | `jsi::Array` | `arr.getValueAtIndex(rt, i)` | `jsi::Array(rt, size)` |
| `null` | - | `jsi::Value` | - | `jsi::Value::null()` |
| `undefined` | - | `jsi::Value` | - | `jsi::Value::undefined()` |

> **重要**: 对于基本类型 (`string`, `number`, `boolean`)，Codegen 会生成对应的 C++ 类型作为返回值，而不是 `jsi::Value`。

### 处理 Object 示例

```typescript
// TypeScript
readonly processUser: (user: {name: string, age: number}) => string;
```

```cpp
// C++ Header
jsi::String processUser(jsi::Runtime& rt, jsi::Object user);

// C++ Implementation
jsi::String NativeSampleModule::processUser(jsi::Runtime& rt, jsi::Object user) {
  // 获取属性
  jsi::Value nameVal = user.getProperty(rt, "name");
  jsi::Value ageVal = user.getProperty(rt, "age");

  // 转换类型
  std::string name = nameVal.asString(rt).utf8(rt);
  double age = ageVal.asNumber();

  // 处理逻辑
  std::string result = "User: " + name + ", Age: " + std::to_string((int)age);

  return jsi::String::createFromUtf8(rt, result);
}
```

### 处理 Array 示例

```typescript
// TypeScript
readonly sumArray: (numbers: number[]) => number;
```

```cpp
// C++ Header
double sumArray(jsi::Runtime& rt, jsi::Array numbers);

// C++ Implementation
double NativeSampleModule::sumArray(jsi::Runtime& rt, jsi::Array numbers) {
  double sum = 0;
  size_t length = numbers.size(rt);

  for (size_t i = 0; i < length; i++) {
    jsi::Value element = numbers.getValueAtIndex(rt, i);
    if (!element.isUndefined()) {
      sum += element.asNumber();
    }
  }

  return sum;
}
```

---

## Expo 项目注意事项

本项目使用 Expo，与标准 React Native 有以下差异：

### 1. Codegen 自动执行

Expo 会在构建时自动运行 codegen，无需手动执行命令：

```bash
# 不需要运行这个命令
# yarn react-native codegen  ❌

# 直接构建即可，codegen 会自动执行
npm run android  ✅
npm run ios       ✅
```

### 2. 项目结构

```
mnn_test/
├── app/                    # Expo 应用代码
│   └── (tabs)/
│       └── native-demo.tsx # TurboModule 演示界面
├── specs/                  # TypeScript 规范
│   └── NativeSampleModule.ts
├── shared/                 # C++ 实现
│   ├── NativeSampleModule.h
│   └── NativeSampleModule.cpp
└── android/
    └── app/src/main/jni/
        └── OnLoad.cpp      # 模块注册
```

### 3. 模块注册

模块已在 `android/app/src/main/jni/OnLoad.cpp` 中注册：

```cpp
std::shared_ptr<TurboModule> cxxModuleProvider(
    const std::string& name,
    const std::shared_ptr<CallInvoker>& jsInvoker) {
  if (name == NativeSampleModule::kModuleName) {
    return std::make_shared<NativeSampleModule>(jsInvoker);
  }
  return autolinking_cxxModuleProvider(name, jsInvoker);
}
```

添加新方法后不需要修改此文件。

---

## 异步方法

### TypeScript 规范

```typescript
export interface Spec extends TurboModule {
  readonly fetchData: (url: string) => Promise<string>;
}
```

### C++ 实现

```cpp
// shared/NativeSampleModule.h
class NativeSampleModule : public NativeSampleModuleCxxSpec<NativeSampleModule> {
public:
  NativeSampleModule(std::shared_ptr<CallInvoker> jsInvoker);

  void fetchData(jsi::Runtime& rt, const std::string& url, const Promise& promise);

private:
  std::shared_ptr<CallInvoker> jsInvoker_;
};
```

```cpp
// shared/NativeSampleModule.cpp
void NativeSampleModule::fetchData(jsi::Runtime& rt, const std::string& url, const Promise& promise) {
  // 在后台线程执行异步操作
  std::thread([this, url, promise]() {
    // 模拟网络请求
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 回到主线程 resolve Promise
    jsInvoker_->invokeAsync([promise, url]() {
      promise.resolve("Data from: " + url);
    });
  }).detach();
}
```

### JavaScript 调用

```typescript
const data = await NativeSampleModule.fetchData('https://api.example.com');
console.log(data);
```

---

## 常见问题

### Q1: Codegen 没有生成最新的代码？

**A**:

**Expo 项目**:
```bash
# 删除缓存并重新构建
rm -rf android/app/build
npm run android
```

**标准 React Native**:
```bash
rm -rf android/app/build
yarn react-native codegen
```

### Q2: 编译时找不到方法定义？

**A**: 确保：
1. TypeScript 规范文件已正确修改
2. Codegen 已运行（Expo: 构建时自动，标准 RN: 手动运行）
3. C++ 头文件和实现文件的方法签名与生成的代码一致
4. 返回类型匹配（`number` → `double`，不是 `jsi::Value`）

### Q3: JavaScript 调用时提示方法未定义？

**A**: 检查：
1. 是否重新编译了 native 代码
2. 是否重新加载了 JavaScript bundle
3. 方法名拼写是否与 TypeScript 规范一致
4. 检查控制台是否有模块加载错误

### Q4: 类型不匹配错误？

**A**: 常见类型错误：
- ❌ `jsi::Value addNumbers(...)` → ✅ `double addNumbers(...)`
- ❌ `jsi::Value greet(...)` → ✅ `jsi::String greet(...)`

**规则**: 检查 Codegen 生成的 `AppSpecsJSI.h` 文件中的虚函数签名，确保你的实现与之匹配。

### Q5: 如何调试 C++ 代码？

**A**:
- Android: 使用 Android Studio 的 LLDB 调试器，在 `shared/NativeSampleModule.cpp` 中设置断点
- iOS: 使用 Xcode 的 LLDB 调试器
- 添加日志输出：
  - Android: `__android_log_print(ANDROID_LOG_INFO, "NativeSampleModule", "Value: %f", value);`
  - iOS: `NSLog(@"Value: %f", value);`

### Q6: 能否传递复杂对象？

**A**: 可以，但需要手动序列化/反序列化：
- 简单对象使用 `jsi::Object`
- 大数据考虑使用 JSON 字符串传递
- 复杂场景考虑使用 JSI 直接操作 JavaScript 对象

---

## 参考资源

- [React Native TurboModule 官方文档](https://reactnative.dev/docs/the-new-architecture/landing-page)
- [JSI API 参考](https://github.com/facebook/react-native/blob/main/packages/react-native/ReactCommon/jsi/jsi/jsi.h)
- [Expo Modules API](https://docs.expo.dev/modules/)
- [本项目的完整示例](../app/(tabs)/native-demo.tsx)
