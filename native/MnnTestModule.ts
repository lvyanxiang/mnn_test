import { NativeModules, NativeModule } from 'react-native';

type MnnTestModuleType = {
  testCall: () => Promise<string>;
};

const { MnnTestModule } = NativeModules;

export default MnnTestModule as MnnTestModuleType;
