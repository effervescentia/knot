import * as path from 'path';
import validateOptions from 'schema-utils';
import createCompiler from './compiler';
import { KNOT_SOURCE_PATTERN } from './constants';
import schema from './schema.json';

const DEFAULT_OPTIONS = {
  port: 1338,
  debug: false,
  config: process.cwd(),
  plugins: {
    jsx: '@knot/react-plugin',
    style: '@knot/css-plugin',
    utils: '@knot/javascript-utils'
  }
};

export default class KnotWebpackPlugin {
  options;

  constructor(options) {
    validateOptions(schema, options, KnotWebpackPlugin.name);

    this.options = {
      ...DEFAULT_OPTIONS,
      ...options
    };
  }

  apply(compiler) {
    const options = this.options;
    const knotCompiler = createCompiler(options);

    function kill() {
      return knotCompiler.close().then(() => process.exit(-1));
    }

    compiler.hooks.beforeRun.tapPromise(KnotWebpackPlugin.name, () =>
      knotCompiler.await().catch(kill)
    );
    compiler.hooks.watchRun.tapPromise(KnotWebpackPlugin.name, () =>
      knotCompiler
        .awaitComplete()
        .then(() => knotCompiler.reset())
        .then(() => knotCompiler.await())
        .catch(kill)
    );

    compiler.hooks.done.tapPromise(KnotWebpackPlugin.name, knotCompiler.close);

    compiler.hooks.compilation.tap(KnotWebpackPlugin.name, compilation => {
      compilation.hooks.succeedModule.tap(
        KnotWebpackPlugin.name,
        discoverDependencies(knotCompiler, kill)
      );

      compilation.hooks.normalModuleLoader.tap(
        KnotWebpackPlugin.name,
        addModuleLoader(knotCompiler, options)
      );
    });
  }
}

function addModuleLoader(knotCompiler, options) {
  return (_, module_) => {
    if (KNOT_SOURCE_PATTERN.test(module_.request)) {
      module_.loaders.unshift({
        loader: path.resolve(__dirname, './loader'),
        options: {
          ...options,
          compiler: knotCompiler
        }
      });
    }
  };
}

function discoverDependencies(knotCompiler, kill) {
  return module_ => {
    const knotDeps = module_.dependencies.filter(({ request }) =>
      KNOT_SOURCE_PATTERN.test(request)
    );

    if (knotDeps.length !== 0) {
      knotDeps.forEach(({ request }) =>
        knotCompiler
          .add(path.resolve(path.dirname(module_.resource), request))
          .catch(kill)
      );
    }
  };
}
