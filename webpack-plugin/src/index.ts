// tslint:disable:no-expression-statement
import * as path from 'path';
import validateOptions from 'schema-utils';
// tslint:disable-next-line:no-implicit-dependencies
import * as Webpack from 'webpack';
import createCompiler from './compiler';
import { KNOT_SOURCE_PATTERN } from './constants';
import schema from './schema.json';
import { Compiler, Options } from './types';

import WebpackCompiler = Webpack.Compiler;
import WebpackCompilation = Webpack.compilation.Compilation;

// tslint:disable-next-line:no-object-literal-type-assertion
export const DEFAULT_OPTIONS = {
  config: process.cwd(),
  debug: false,
  plugins: {
    jsx: '@knot/react-plugin',
    style: '@knot/css-plugin',
    utils: '@knot/javascript-utils'
  },
  port: 1338
} as Options;

export default class KnotWebpackPlugin {
  public readonly options: Options;

  constructor(options: Partial<Options>) {
    validateOptions(schema, options, KnotWebpackPlugin.name);

    this.options = {
      ...DEFAULT_OPTIONS,
      ...options
    };
  }

  public apply(compiler: WebpackCompiler): void {
    const options = this.options;
    const knotCompiler = createCompiler(options);
    const knotLoader = {
      loader: path.resolve(__dirname, './loader'),
      options: {
        ...options,
        compiler: knotCompiler
      }
    };
    // tslint:disable-next-line:no-let
    let successiveRun = false;
    // tslint:disable-next-line:no-let
    let watching = false;

    const kill = async (): Promise<void> => {
      await knotCompiler.close();

      process.exit(-1);
    };

    compiler.hooks.watchRun.tapPromise(KnotWebpackPlugin.name, () =>
      (successiveRun
        ? Promise.resolve()
        : // tslint:disable-next-line:ban-comma-operator
          ((watching = true), knotCompiler.await())
      ).catch(kill)
    );
    compiler.hooks.beforeRun.tapPromise(KnotWebpackPlugin.name, () =>
      watching ? Promise.resolve() : knotCompiler.await().catch(kill)
    );

    compiler.hooks.done.tapPromise(KnotWebpackPlugin.name, () => {
      if (watching) {
        successiveRun = true;

        return Promise.resolve();
      } else {
        return knotCompiler.close();
      }
    });

    compiler.hooks.invalid.tap(
      KnotWebpackPlugin.name,
      invalidateModule(knotCompiler, kill)
    );

    compiler.hooks.normalModuleFactory.tap(KnotWebpackPlugin.name, nmf => {
      nmf.hooks.beforeResolve.tap(KnotWebpackPlugin.name, mod => {
        if (!mod) {
          return;
        }

        if (mod.request.startsWith('@knot/')) {
          const [, plugin] = mod.request.split('/');

          if (plugin in options.plugins) {
            mod.request = options.plugins[plugin];
          }
        }

        return mod;
      });
    });

    compiler.hooks.compilation.tap(
      KnotWebpackPlugin.name,
      (compilation: WebpackCompilation) => {
        compilation.hooks.succeedModule.tap(
          KnotWebpackPlugin.name,
          discoverDependencies(knotCompiler, kill)
        );

        compilation.hooks.normalModuleLoader.tap(
          KnotWebpackPlugin.name,
          addModuleLoader(knotLoader)
        );
      }
    );
  }
}

function invalidateModule(
  knotCompiler: Compiler,
  kill: () => Promise<void>
): (path: string) => void {
  return invalidPath =>
    KNOT_SOURCE_PATTERN.test(invalidPath) &&
    knotCompiler.invalidate(invalidPath).catch(kill);
}

function addModuleLoader(knotLoader): (_: any, mod: any) => void {
  return (_, mod) => {
    if (
      KNOT_SOURCE_PATTERN.test(mod.request) &&
      !mod.loaders.includes(knotLoader)
    ) {
      mod.loaders.unshift(knotLoader);
    }
  };
}

function discoverDependencies(
  knotCompiler: Compiler,
  kill: (e: Error) => void
): (mod: any) => void {
  return mod => {
    const knotDeps = mod.dependencies.filter(({ request }) =>
      KNOT_SOURCE_PATTERN.test(request)
    );

    if (knotDeps.length !== 0) {
      knotDeps.forEach(({ request }) =>
        knotCompiler
          .add(path.resolve(path.dirname(mod.resource), request))
          .catch(kill)
      );
    }
  };
}
