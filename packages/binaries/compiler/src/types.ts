import { DeepPartial } from 'utility-types';

export enum ServerStatus {
  IDLE = 'idle',
  COMPLETE = 'complete'
}

export enum ModuleStatus {
  PENDING = 'pending',
  COMPLETE = 'complete',
  FAILED = 'failed'
}

export interface Options {
  readonly port: number;
  readonly debug: boolean;
  readonly knot: string;
  readonly config: string;
  readonly plugins: Plugins;
  readonly target: 'javascript-es6' | 'javascript-common';
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
