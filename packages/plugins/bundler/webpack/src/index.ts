// tslint:disable:no-expression-statement
import * as path from 'path';
import validateOptions from 'schema-utils';
import * as Webpack from 'webpack';
import createCompiler from './compiler';
import HOOKS from './hooks';
import schema from './schema.json';
import { Compiler, Context, Options } from './types';
import { createTerminator } from './utils';

import WebpackCompiler = Webpack.Compiler;

// tslint:disable-next-line:no-object-literal-type-assertion
export const DEFAULT_OPTIONS = {
  config: process.cwd(),
  debug: false,
  plugins: {
    jsx: '@knot/react-plugin',
    platform: '@knot/browser-plugin',
    style: '@knot/jss-plugin'
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
    const context = createContext(options, knotCompiler);
    const kill = createTerminator(knotCompiler);

    HOOKS.forEach(hook => hook(compiler, context, kill));
  }
}

function createContext(options: Options, knotCompiler: Compiler): Context {
  return {
    knotCompiler,
    knotLoader: {
      loader: path.resolve(__dirname, './loader'),
      options: {
        ...options,
        compiler: knotCompiler
      }
    },
    name: KnotWebpackPlugin.name,
    options,
    successiveRun: false,
    watching: false
  };
}
