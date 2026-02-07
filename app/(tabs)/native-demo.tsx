import NativeSampleModule from '@/specs/NativeSampleModule';
import React, { useState } from 'react';
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
  debug: {
    marginTop: 20,
    fontSize: 10,
    color: '#666',
  },
});
