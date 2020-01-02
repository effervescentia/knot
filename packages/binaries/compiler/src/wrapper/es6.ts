import { ModuleWrapper } from '../types';

const ES6Wrapper: ModuleWrapper = {
  genMainImport(name: string, mod: string): string {
    return `import {main as ${name}} from "${mod}";`;
  },

  genModuleImport(name: string, mod: string): string {
    return `import * as ${name} from "${mod}";`;
  }
};

export default ES6Wrapper;
