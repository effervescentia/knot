import { DeepPartial } from 'utility-types';

import { ClientOptions } from './compiler/client';

export enum Target {
  JAVASCRIPT_ES6 = 'javascript-es6',
  JAVASCRIPT_COMMON = 'javascript-common',
}

export interface Options extends ClientOptions {
  readonly plugins: Plugins;
}

export type OptionOverrides = DeepPartial<Options>;

export interface Plugins {
  readonly jsx: string;
  readonly style: string;
  readonly platform: string;
}

export interface ModuleWrapper {
  readonly genMainImport: (name: string, mod: string) => string;

  readonly genModuleImport: (name: string, mod: string) => string;
}
