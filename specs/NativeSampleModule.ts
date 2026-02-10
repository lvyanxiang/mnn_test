import {TurboModule, TurboModuleRegistry} from 'react-native';

export interface Spec extends TurboModule {
  readonly reverseString: (input: string) => string;
  readonly addNumbers: (a: number, b: number) => number;
  readonly initFaceDetector: () => string;  // 无需传参数，模型自动从 assets 加载
  readonly detectFace: (imagePath: string) => string;
}

export default TurboModuleRegistry.getEnforcing<Spec>(
  'NativeSampleModule',
);
