// tslint:disable:no-expression-statement
import * as path from 'path';
import validateOptions from 'schema-utils';
import * as Webpack from 'webpack';
import createCompiler from './compiler';
import HOOKS from './hooks';
import schema from './schema.json';
import { Options } from './types';

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
    const knotLoader = {
      loader: path.resolve(__dirname, './loader'),
      options: {
        ...options,
        compiler: knotCompiler
      }
    };
    const context = {
      knotCompiler,
      knotLoader,
      name: KnotWebpackPlugin.name,
      options,
      successiveRun: false,
      watching: false
    };

    const kill = async (): Promise<void> => {
      await knotCompiler.close();

      process.exit(-1);
    };

    HOOKS.forEach(hook => hook(compiler, context, kill));
  }
}
