import KnotCompiler, { isKnot, Options } from '@knot/compiler';
import * as path from 'path';
import * as Webpack from 'webpack';

import { Kill } from './types';

import WebpackModule = Webpack.loader.LoaderContext;

export function createTerminator(
  knotCompiler: KnotCompiler
): (err: Error) => Promise<void> {
  return async err => {
    console.error('compilation failed with error: ', err);
    console.log('waiting for compiler to shut down...');
    await knotCompiler.close();
    console.log('compiler shut down successfully');

    process.exit(-1);
  };
}

export function resolveLibrary(
  options: Options
): (mod: WebpackModule) => WebpackModule {
  return mod => {
    if (!mod) {
      return mod;
    }

    if (mod.request.startsWith('@knot/')) {
      const [, plugin] = mod.request.split('/');

      if (plugin in options.plugins) {
        mod.request = options.plugins[plugin];
      }
    }

    return mod;
  };
}

export function invalidateModule(
  knotCompiler: KnotCompiler,
  kill: Kill
): (path: string) => void {
  return invalidPath =>
    isKnot(invalidPath) && knotCompiler.invalidate(invalidPath).catch(kill);
}

export function addModuleLoader(
  knotLoader: Webpack.Loader
): (_: any, mod: WebpackModule) => void {
  return (_, mod) => {
    if (isKnot(mod.request) && !mod.loaders.includes(knotLoader)) {
      mod.loaders.unshift(knotLoader);
    }
  };
}

export function discoverDependencies(
  knotCompiler: KnotCompiler,
  kill: Kill
): (mod: any) => Promise<any> {
  return async mod => {
    const knotDeps = mod.dependencies.filter(({ request }) => isKnot(request));

    if (knotDeps.length === 0) {
      return Promise.resolve();
    }

    try {
      return Promise.all(
        knotDeps.map(({ request }) =>
          knotCompiler.add(path.resolve(path.dirname(mod.resource), request))
        )
      );
    } catch (e) {
      return kill(e);
    }
  };
}
