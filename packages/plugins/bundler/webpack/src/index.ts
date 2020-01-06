import KnotCompiler, { Options } from '@knot/compiler';
import * as path from 'path';
import validateOptions from 'schema-utils';
import * as Webpack from 'webpack';

import HOOKS from './hooks';
import schema from './schema.json';
import { Context } from './types';
import { createTerminator } from './utils';

import WebpackCompiler = Webpack.Compiler;

export default class KnotWebpackPlugin {
  constructor(public options: Partial<Options>) {
    validateOptions(schema, options, KnotWebpackPlugin.name);
  }

  public apply(compiler: WebpackCompiler): void {
    const knotCompiler = new KnotCompiler(this.options);
    // eslint-disable-next-line @typescript-eslint/no-use-before-define
    const context = createContext(knotCompiler);
    const kill = createTerminator(knotCompiler);

    HOOKS.forEach(hook => hook(compiler, context, kill));
  }
}

function createContext(knotCompiler: KnotCompiler): Context {
  return {
    knotCompiler,
    knotLoader: {
      loader: path.resolve(__dirname, './loader'),
      options: {
        ...knotCompiler.options,
        compilerInstance: knotCompiler
      }
    },
    name: KnotWebpackPlugin.name,
    options: knotCompiler.options,
    successiveRun: false,
    watching: false
  };
}
