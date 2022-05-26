import KnotCompiler, { OptionOverrides, Target } from '@knot/compiler';
import * as path from 'path';
import validateOptions from 'schema-utils';
import * as Webpack from 'webpack';

import HOOKS from './hooks';
import schema from './schema.json';
import { Context } from './types';
import { createTerminator } from './utils';

import WebpackCompiler = Webpack.Compiler;
import { JSONSchema7 } from 'schema-utils/declarations/validate';

export default class KnotWebpackPlugin {
  public options: OptionOverrides;

  constructor(options: OptionOverrides = {}) {
    const configuredOptions: OptionOverrides = {
      ...options,
      target: Target.JAVASCRIPT_COMMON,
    };

    validateOptions(schema as JSONSchema7, configuredOptions, {
      name: KnotWebpackPlugin.name,
    });

    this.options = configuredOptions;
  }

  public apply(compiler: WebpackCompiler): void {
    const knotCompiler = new KnotCompiler(this.options);
    // eslint-disable-next-line @typescript-eslint/no-use-before-define
    const context = createContext(knotCompiler);
    const kill = createTerminator(knotCompiler);

    HOOKS.forEach((hook) => hook(compiler, context, kill));
  }
}

function createContext(knotCompiler: KnotCompiler): Context {
  return {
    knotCompiler,
    knotLoader: {
      loader: path.resolve(__dirname, './loader'),
      options: {
        ...knotCompiler.options,
        compilerInstance: knotCompiler,
      },
    },
    name: KnotWebpackPlugin.name,
    options: knotCompiler.options,
    successiveRun: false,
    watching: false,
  };
}
