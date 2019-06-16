// tslint:disable:no-expression-statement
import * as path from 'path';
import * as Webpack from 'webpack';
import { KNOT_SOURCE_PATTERN } from './constants';
import { Compiler, Kill, Options } from './types';

import WebpackModule = Webpack.loader.LoaderContext;

export function createTerminator(
  knotCompiler: Compiler
): (err: Error) => Promise<void> {
  return async err => {
    // tslint:disable: no-console
    console.error('compilation failed with error: ', err);
    console.log('waiting for compiler to shut down...');
    await knotCompiler.close();
    console.log('compiler shut down successfully');
    // tslint:enable: no-console

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
        // tslint:disable-next-line:no-object-mutation
        mod.request = options.plugins[plugin];
      }
    }

    return mod;
  };
}

export function invalidateModule(
  knotCompiler: Compiler,
  kill: Kill
): (path: string) => void {
  return invalidPath =>
    KNOT_SOURCE_PATTERN.test(invalidPath) &&
    knotCompiler.invalidate(invalidPath).catch(kill);
}

export function addModuleLoader(
  knotLoader: Webpack.Loader
): (_: any, mod: WebpackModule) => void {
  return (_, mod) => {
    if (
      KNOT_SOURCE_PATTERN.test(mod.request) &&
      !mod.loaders.includes(knotLoader)
    ) {
      mod.loaders.unshift(knotLoader);
    }
  };
}

export function discoverDependencies(
  knotCompiler: Compiler,
  kill: Kill
): (mod: any) => Promise<any> {
  return async mod => {
    const knotDeps = mod.dependencies.filter(({ request }) =>
      KNOT_SOURCE_PATTERN.test(request)
    );

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
