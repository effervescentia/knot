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
  readonly compiler: {
    readonly module: 'es6' | 'common';
  };
}

export interface Plugins {
  readonly jsx: string;
  readonly style: string;
  readonly platform: string;
}
