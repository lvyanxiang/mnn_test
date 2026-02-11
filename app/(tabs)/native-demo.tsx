import NativeSampleModule from '@/specs/NativeSampleModule';
import { Asset } from 'expo-asset';
import React, { useEffect, useState } from 'react';
import {
  Button,
  SafeAreaView,
  StyleSheet,
  Text,
  TextInput,
  View,
} from 'react-native';

export default function NativeDemoScreen() {
  const [value, setValue] = useState('Hello');
  const [reversedValue, setReversedValue] = useState('');
  const [debugInfo, setDebugInfo] = useState('');

  // Add numbers demo state
  const [numA, setNumA] = useState('10');
  const [numB, setNumB] = useState('20');
  const [sumResult, setSumResult] = useState('');

  // Face detection demo state
  const [initResult, setInitResult] = useState('');
  const [imagePath, setImagePath] = useState('');
  const [faceResult, setFaceResult] = useState('');

  // 获取测试图片的本地路径
  useEffect(() => {
    async function loadImagePath() {
      try {
        const asset = Asset.fromModule(require('../../assets/images/face.jpg'));
        await asset.downloadAsync(); // 确保文件已下载
        console.log('Test image localUri:', asset.localUri);

        // Asset.localUri 格式为 "file:///path/to/file"
        // 需要转换为真实文件系统路径 "/path/to/file"
        let fsPath = asset.localUri || '';
        if (fsPath.startsWith('file://')) {
          fsPath = fsPath.substring(7); // 移除 "file://" 前缀
        }
        console.log('Test image filesystem path:', fsPath);
        setImagePath(fsPath);
      } catch (error) {
        console.error('Failed to load test image:', error);
      }
    }
    loadImagePath();
  }, []);

  const onPress = () => {
    try {
      console.log('=== Debug Info ===');
      console.log('Module:', NativeSampleModule);
      console.log('Module keys:', Object.keys(NativeSampleModule));
      console.log('reverseString type:', typeof NativeSampleModule.reverseString);
      console.log('Input value:', value);

      const result = NativeSampleModule.reverseString(value);
      console.log('Result:', result);
      console.log('Result type:', typeof result);

      setDebugInfo(JSON.stringify({
        moduleKeys: Object.keys(NativeSampleModule),
        hasReverseString: typeof NativeSampleModule.reverseString === 'function',
        input: value,
        result: result,
        resultType: typeof result,
      }, null, 2));

      setReversedValue(result || '<empty or undefined>');
    } catch (error) {
      console.error('Error calling reverseString:', error);
      setDebugInfo('Error: ' + String(error));
      setReversedValue('Error: ' + String(error));
    }
  };

  const onAddNumbersPress = () => {
    try {
      const a = parseFloat(numA) || 0;
      const b = parseFloat(numB) || 0;

      console.log('=== Add Numbers Debug ===');
      console.log('Input a:', a, 'Type:', typeof a);
      console.log('Input b:', b, 'Type:', typeof b);

      const result = NativeSampleModule.addNumbers(a, b);
      console.log('Result:', result, 'Type:', typeof result);

      setSumResult(String(result));
    } catch (error) {
      console.error('Error calling addNumbers:', error);
      setSumResult('Error: ' + String(error));
    }
  };

  const onInitDetectorPress = () => {
    try {
      console.log('=== Init Face Detector ===');
      // 不再需要传递模型路径，自动从 assets 加载
      const result = NativeSampleModule.initFaceDetector();
      console.log('Result:', result);

      setInitResult(result);
    } catch (error) {
      console.error('Error calling initFaceDetector:', error);
      setInitResult('Error: ' + String(error));
    }
  };

  const onDetectFacePress = () => {
    try {
      console.log('=== Face Detection Debug ===');
      console.log('Image path:', imagePath);

      const result = NativeSampleModule.detectFace(imagePath);
      console.log('Result:', result);

      setFaceResult(result);
    } catch (error) {
      console.error('Error calling detectFace:', error);
      setFaceResult('Error: ' + String(error));
    }
  };

  return (
    <SafeAreaView style={styles.container}>
      <View>
        <Text style={styles.title}>
          C++ Turbo Native Module Demo
        </Text>
        <Text>Write down the text you want to reverse:</Text>
        <TextInput
          style={styles.textInput}
          placeholder="Write your text here"
          onChangeText={setValue}
          value={value}
        />
        <Button title="Reverse" onPress={onPress} />
        <Text style={styles.result}>Reversed text: {reversedValue}</Text>

        <View style={styles.separator} />

        <Text style={styles.sectionTitle}>Add Numbers Demo (C++)</Text>
        <View style={styles.numberInputContainer}>
          <TextInput
            style={styles.numberInput}
            placeholder="Number A"
            onChangeText={setNumA}
            value={numA}
            keyboardType="numeric"
          />
          <Text style={styles.plusSign}>+</Text>
          <TextInput
            style={styles.numberInput}
            placeholder="Number B"
            onChangeText={setNumB}
            value={numB}
            keyboardType="numeric"
          />
        </View>
        <Button title="Add Numbers" onPress={onAddNumbersPress} />
        <Text style={styles.result}>Sum result: {sumResult}</Text>

        <View style={styles.separator} />

        <Text style={styles.sectionTitle}>Face Detection Demo (C++)</Text>
        <Text style={styles.hint}>Step 1: Initialize detector (model auto-loads from assets):</Text>
        <Button title="Initialize Detector" onPress={onInitDetectorPress} />
        <Text style={styles.result}>Init result: {initResult || 'Not initialized'}</Text>

        <Text style={styles.hint}>Step 2: Enter image path to detect faces:</Text>
        <TextInput
          style={styles.textInput}
          placeholder="/sdcard/test.jpg"
          onChangeText={setImagePath}
          value={imagePath}
        />
        <Button title="Detect Face" onPress={onDetectFacePress} />
        <Text style={styles.result}>Detection result:</Text>
        <Text style={styles.faceResult}>{faceResult || 'No result yet'}</Text>

        <Text style={styles.debug}>Debug info:{'\n'}{debugInfo}</Text>
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
  title: {
    fontSize: 18,
    marginBottom: 20,
    fontWeight: 'bold',
  },
  textInput: {
    borderColor: '#000',
    borderWidth: 1,
    borderRadius: 5,
    padding: 10,
    marginTop: 10,
    minWidth: 200,
  },
  result: {
    marginTop: 20,
    fontSize: 16,
  },
  faceResult: {
    marginTop: 10,
    fontSize: 12,
    color: '#333',
    textAlign: 'center',
    paddingHorizontal: 20,
  },
  hint: {
    fontSize: 12,
    color: '#666',
    marginBottom: 5,
    marginTop: 10,
  },
  debug: {
    marginTop: 20,
    fontSize: 10,
    color: '#666',
  },
  separator: {
    height: 1,
    backgroundColor: '#ccc',
    width: '80%',
    marginVertical: 20,
  },
  sectionTitle: {
    fontSize: 16,
    marginBottom: 10,
    fontWeight: '600',
  },
  numberInputContainer: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    marginVertical: 10,
  },
  numberInput: {
    borderColor: '#000',
    borderWidth: 1,
    borderRadius: 5,
    padding: 10,
    width: 100,
    textAlign: 'center',
  },
  plusSign: {
    fontSize: 24,
    marginHorizontal: 10,
  },
});
