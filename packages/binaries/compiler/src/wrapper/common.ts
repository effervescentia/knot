import { ModuleWrapper } from '../types';

const CommonWrapper: ModuleWrapper = {
  genMainImport(name: string, mod: string): string {
    return `var ${name} = require("${mod}").main;`;
  },

  genModuleImport(name: string, mod: string): string {
    return `var ${name} = require("${mod}");`;
  }
};

export default CommonWrapper;
