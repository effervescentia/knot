// tslint:disable:no-expression-statement
import * as Webpack from 'webpack';
import { Context } from './types';
import {
  addModuleLoader,
  discoverDependencies,
  invalidateModule,
  resolveLibrary
} from './utils';

import WebpackCompiler = Webpack.Compiler;
import WebpackCompilation = Webpack.compilation.Compilation;

export interface Hook {
  (compiler: WebpackCompiler, context: Context, kill: () => Promise<void>);
}

export function watchCompilationHook(
  compiler: WebpackCompiler,
  context: Context,
  kill: () => Promise<void>
): void {
  compiler.hooks.watchRun.tapPromise(context.name, () =>
    (context.successiveRun
      ? Promise.resolve()
      : // tslint:disable-next-line: ban-comma-operator no-object-mutation
        ((context.watching = true), context.knotCompiler.await())
    ).catch(kill)
  );
}

export function awaitCompilerHook(
  compiler: WebpackCompiler,
  context: Context,
  kill: () => Promise<void>
): void {
  compiler.hooks.beforeRun.tapPromise(context.name, () =>
    context.watching
      ? Promise.resolve()
      : context.knotCompiler.await().catch(kill)
  );
}

export function terminationHook(
  compiler: WebpackCompiler,
  context: Context
): void {
  compiler.hooks.done.tapPromise(context.name, () => {
    if (context.watching) {
      // tslint:disable-next-line: no-object-mutation
      context.successiveRun = true;

      return Promise.resolve();
    } else {
      return context.knotCompiler.close();
    }
  });
}

export function invalidationHook(
  compiler: WebpackCompiler,
  { name, knotCompiler }: Context,
  kill: () => Promise<void>
): void {
  compiler.hooks.invalid.tap(name, invalidateModule(knotCompiler, kill));
}

export function resolutionHook(
  compiler: WebpackCompiler,
  { name, options }: Context
): void {
  compiler.hooks.normalModuleFactory.tap(name, nmf => {
    nmf.hooks.beforeResolve.tap(name, resolveLibrary(options));
  });
}

export function compilationHook(
  compiler: WebpackCompiler,
  { name, knotCompiler, knotLoader }: Context,
  kill: () => Promise<void>
): void {
  compiler.hooks.compilation.tap(name, (compilation: WebpackCompilation) => {
    compilation.hooks.succeedModule.tap(
      name,
      discoverDependencies(knotCompiler, kill)
    );

    compilation.hooks.normalModuleLoader.tap(name, addModuleLoader(knotLoader));
  });
}

// tslint:disable-next-line: readonly-array
const HOOKS: Hook[] = [
  watchCompilationHook,
  awaitCompilerHook,
  terminationHook,
  invalidationHook,
  resolutionHook,
  compilationHook
];

export default HOOKS;
