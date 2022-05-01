import { DeepPartial } from 'utility-types';

export enum Target {
  JAVASCRIPT_ES6 = 'javascript-es6',
  JAVASCRIPT_COMMON = 'javascript-common'
}

export interface Options {
  readonly port: number;
  readonly debug: boolean;
  readonly knotc: string;
  readonly rootDir: string;
  readonly config: string;
  readonly plugins: Plugins;
  readonly target: Target;
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
