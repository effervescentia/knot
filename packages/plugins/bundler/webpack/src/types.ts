import KnotCompiler, { Options } from '@knot/compiler';

export type Kill = (err: Error) => Promise<void>;

export interface Loader {
  readonly loader: string;
  readonly options: InternalOptions;
}

export interface InternalOptions extends Options {
  readonly compilerInstance: KnotCompiler;
}

export interface Context {
  // tslint:disable: readonly-keyword
  successiveRun: boolean;
  watching: boolean;
  // tslint:enable: readonly-keyword

  readonly name: string;
  readonly options: Options;
  readonly knotCompiler: KnotCompiler;
  readonly knotLoader: Loader;
}
