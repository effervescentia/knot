import { resolveLibrary } from '@knot/compiler';
import * as Webpack from 'webpack';

import { Context, Kill } from './types';
import {
  addModuleLoader,
  discoverDependencies,
  invalidateModule,
} from './utils';

import WebpackCompiler = Webpack.Compiler;
import WebpackCompilation = Webpack.compilation.Compilation;

export type Hook = (
  compiler: WebpackCompiler,
  context: Context,
  kill: Kill
) => void;

export function watchCompilationHook(
  compiler: WebpackCompiler,
  context: Context,
  kill: Kill
): void {
  compiler.hooks.watchRun.tapPromise(context.name, async () => {
    if (context.successiveRun) return Promise.resolve();

    context.watching = true;

    await context.knotCompiler.start().catch(kill);
  });
}

export function awaitCompilerHook(
  compiler: WebpackCompiler,
  context: Context,
  kill: Kill
): void {
  compiler.hooks.beforeRun.tapPromise(context.name, () =>
    context.watching
      ? Promise.resolve()
      : context.knotCompiler.awaitReady().catch(kill)
  );
}

export function terminationHook(
  compiler: WebpackCompiler,
  context: Context
): void {
  compiler.hooks.done.tap(context.name, () => {
    if (context.watching) {
      context.successiveRun = true;
    } else {
      context.knotCompiler.close();
    }
  });
}

export function invalidationHook(
  compiler: WebpackCompiler,
  { name, knotCompiler }: Context,
  kill: Kill
): void {
  compiler.hooks.invalid.tap(name, invalidateModule(knotCompiler, kill));
}

export function resolutionHook(
  compiler: WebpackCompiler,
  { name, options }: Context
): void {
  compiler.hooks.normalModuleFactory.tap(name, (nmf) => {
    nmf.hooks.beforeResolve.tap(name, (mod) => {
      const resolved = resolveLibrary(mod.request, options);
      if (resolved) {
        mod.request = resolved;
      }

      return mod;
    });
  });
}

export function compilationHook(
  compiler: WebpackCompiler,
  { name, knotCompiler, knotLoader }: Context,
  kill: Kill
): void {
  compiler.hooks.compilation.tap(name, (compilation: WebpackCompilation) => {
    compilation.hooks.succeedModule.tap(
      name,
      discoverDependencies(knotCompiler, kill)
    );

    compilation.hooks.normalModuleLoader.tap(name, addModuleLoader(knotLoader));
  });
}

const HOOKS: Readonly<Hook[]> = [
  watchCompilationHook,
  awaitCompilerHook,
  terminationHook,
  invalidationHook,
  resolutionHook,
  compilationHook,
];

export default HOOKS;
