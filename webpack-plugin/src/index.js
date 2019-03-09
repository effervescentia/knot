const path = require('path');
const validateOptions = require('schema-utils');
const {
  stringify
} = require('javascript-stringify');
const createCompiler = require('./compiler');
const schema = require('./schema.json');

const DEFAULT_OPTIONS = {
  port: 1338,
  config: process.cwd()
};
const KNOT_SOURCE_PATTERN = /.*\.kn(ot)?$/;

module.exports = class KnotWebpackLoader {
  constructor(options) {
    validateOptions(schema, options, KnotWebpackLoader.name);

    this.options = {
      ...DEFAULT_OPTIONS,
      ...options
    };
  }

  apply(compiler) {
    const knotCompiler = createCompiler(this.options);

    // function awaitCompiler() {
    //   console.error('BEFORE RUN - COMPILE ALL KNOT FILES');

    //   return knotCompiler.await();
    // }

    // compiler.hooks.beforeCompile.tap(KnotWebpackLoader.name, console.log.bind(null, compiler, '\n'));

    // compiler.hooks.beforeRun.tapPromise(KnotWebpackLoader.name, awaitCompiler);
    // compiler.hooks.watchRun.tapPromise(KnotWebpackLoader.name, awaitCompiler);

    // compiler.hooks.beforeCompile.tapPromise(KnotWebpackLoader.name, (compiler) => {
    //   console.error('BEFORE COMPILE - REPLACE ALL KNOT FILES');
    //   console.log(stringify(compiler, null, null, {
    //     maxDepth: 1
    //   }));
    //   // console.log(stringify(compiler), null, null, {
    //   //   maxDepth: 1
    //   // });

    //   return knotCompiler.await();
    // });

    // compiler.hooks.normalModuleFactory.tap(KnotWebpackLoader.name, (nmf) => {
    //   console.error('NORMAL MODULE FACTORY');

    //   console.log(nmf);
    // });

    // compiler.hooks.contextModuleFactory.tap(KnotWebpackLoader.name, (cmf) => {
    //   console.error('CONTEXT MODULE FACTORY');

    //   console.log(cmf);
    // });

    compiler.hooks.compilation.tap(KnotWebpackLoader.name, (compilation) => {
      compilation.hooks.succeedModule.tap(KnotWebpackLoader.name, (module_) => {
        const knotDeps = module_.dependencies.filter(({
          request
        }) => KNOT_SOURCE_PATTERN.test(request));

        if (knotDeps.length !== 0) {
          knotDeps.forEach(({
            request
          }) => knotCompiler.add(path.resolve(path.dirname(module_.resource), request)));
        }
      });
      // compilation.hooks.succeedModule.tap(KnotWebpackLoader.name, (module_) => module_.resource === '/Users/benteichman/development/knot/examples/webpack-react/src/index.js' ? console.log(module_.dependencies[0].request) : null);

      // compilation.hooks.succeedModule.tap(KnotWebpackLoader.name, (module_) => module_.resource === '/Users/benteichman/development/knot/examples/webpack-react/src/index.js' ? console.log(module_.dependencies[0]) : null);
      // compilation.hooks.succeedModule.tap(KnotWebpackLoader.name, (module_) => times === 2 ? null : (Object.keys(module_).forEach(key => {
      //   console.log(`\n${key}:`);
      //   if (module_[key] && typeof module_[key] === 'object') {
      //     Object.keys(module_[key]).forEach(ckey => console.log(`\t${ckey}: ${module_[key][ckey]}`));
      //   } else {
      //     console.log(`\t${module_[key]}`)
      //   }
      // }), times++));

      // tap('buildModule');
      // tap('rebuildModule');
      // tap('failedModule');
      // tap('succeedModule');
      // tap('finishModules');
      // tap('finishRebuildingModule');
      // tap('seal');
      // tap('unseal');
      // tap('optimizeDependenciesBasic');
      // tap('optimizeDependencies');
      // tap('optimizeDependenciesAdvanced');
      // tap('afterOptimizeDependencies');
      // tap('optimize');
      // tap('optimizeModulesBasic');
      // tap('optimizeModules');
      // tap('optimizeModulesAdvanced');
      // tap('afterOptimizeModules');
      // tap('optimizeChunksBasic');
      // tap('optimizeChunks');
      // tap('optimizeChunksAdvanced');
      // tap('afterOptimizeChunks');
      // tap('optimizeTree');
      // tap('afterOptimizeTree');
      // tap('optimizeChunkModulesBasic');
      // tap('optimizeChunkModules');
      // tap('optimizeChunkModulesAdvanced');
      // tap('afterOptimizeChunkModules');
      // tap('shouldRecord');
      // tap('reviveModules');
      // tap('optimizeModuleOrder');
      // tap('advancedOptimizeModuleOrder');
      // tap('beforeModuleIds');
      // tap('moduleIds');
      // tap('optimizeModuleIds');
      // tap('afterOptimizeModuleIds');
      // tap('reviveChunks');
      // tap('optimizeChunkOrder');
      // // tap('beforeOptimizeChunkIds');
      // tap('optimizeChunkIds');
      // tap('afterOptimizeChunkIds');
      // tap('recordModules');
      // tap('recordChunks');
      // tap('beforeHash');
      // tap('afterHash');
      // tap('recordHash');
      // tap('record');
      // tap('beforeModuleAssets');
      // tap('shouldGenerateChunkAssets');
      // tap('beforeChunkAssets');
      // tap('additionalChunkAssets');
      // // tap('records');
      // tap('additionalAssets');
      // tap('optimizeChunkAssets');
      // tap('afterOptimizeChunkAssets');
      // tap('optimizeAssets');
      // tap('afterOptimizeAssets');
      // tap('needAdditionalSeal');
      // tap('afterSeal');
      // tap('chunkHash');
      // tap('moduleAsset');
      // tap('chunkAsset');
      // tap('assetPath');
      // tap('needAdditionalPass');
      // tap('childCompiler');
      // tap('normalModuleLoader');
      // tap('dependencyReference');
      // console.log(Object.keys(compilation));

      // compilation.hooks.normalModuleLoader.tap(KnotWebpackLoader.name, (nml, module_) => {
      //   console.error('LOAD MODULE - ' + module_.request);

      //   if (/.*\.kn(ot)?$/.test(module_.request)) {
      //     module_.loaders.unshift({
      //       loader: path.resolve(__dirname, './loader.js'),
      //       options: {
      //         compiler: knotCompiler
      //       }
      //     });
      //   }
      // });
    });
  }
};