import React, { useState } from 'react';
import { View, Text, StyleSheet, Button, Alert } from 'react-native';
import MnnTestModule from '../native/MnnTestModule';

export default function NativeTestScreen() {
  const [result, setResult] = useState<string>('');

  const handleTestCall = async () => {
    try {
      const response = await MnnTestModule.testCall();
      setResult(response);
      Alert.alert('Success', response);
    } catch (error) {
      setResult(`Error: ${error}`);
      Alert.alert('Error', String(error));
    }
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Native Module Test</Text>

      <View style={styles.buttonContainer}>
        <Button
          title="Test Native Call"
          onPress={handleTestCall}
        />
      </View>

      {result ? (
        <View style={styles.resultContainer}>
          <Text style={styles.resultTitle}>Result:</Text>
          <Text style={styles.resultText}>{result}</Text>
        </View>
      ) : null}
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 20,
    backgroundColor: '#fff',
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 20,
    textAlign: 'center',
  },
  buttonContainer: {
    marginBottom: 20,
  },
  resultContainer: {
    padding: 15,
    backgroundColor: '#f0f0f0',
    borderRadius: 8,
  },
  resultTitle: {
    fontSize: 16,
    fontWeight: '600',
    marginBottom: 8,
  },
  resultText: {
    fontSize: 14,
    color: '#333',
  },
});
