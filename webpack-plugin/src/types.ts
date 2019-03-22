// tslint:disable-next-line:ban-types
export type ArgumentTypes<F extends Function> = F extends (
  ...args: infer A
) => any
  ? A
  : never;

export enum ServerStatus {
  IDLE = 'idle',
  COMPLETE = 'complete'
}

export enum ModuleStatus {
  PENDING = 'pending',
  COMPLETE = 'complete'
}

export interface Compiler {
  readonly await: () => Promise<void>;
  readonly awaitComplete: () => Promise<void>;
  readonly awaitModule: (path: string) => Promise<void>;
  readonly generate: (path: string) => Promise<void | string>;
  readonly add: (path: string) => Promise<void | Response>;
  readonly invalidate: (path: string) => Promise<void | Response>;
  readonly close: () => Promise<void | Response>;
}

export interface Options {
  readonly port: number;
  readonly debug: boolean;
  readonly knot: string;
  readonly config: string;
  readonly plugins: Plugins;
}

export interface InternalOptions extends Options {
  readonly compiler: Compiler;
}

export interface Plugins {
  readonly jsx: string;
  readonly style: string;
  readonly utils: string;
}
