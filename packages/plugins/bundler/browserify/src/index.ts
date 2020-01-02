// tslint:disable: no-expression-statement
import KnotCompiler, { Options } from '@knot/compiler';
import { BrowserifyObject } from 'browserify';
import setupPipeline from './setup';
import transformFile from './transform';

function browserifyKnot(
  browser: BrowserifyObject,
  options: Partial<Options>
): void {
  const configuredOptions: Partial<Options> = {
    ...options,
    compiler: {
      ...options.compiler,
      module: 'common'
    }
  };

  const compiler = new KnotCompiler(configuredOptions);

  setupPipeline(browser, compiler.options);

  browser.transform(transformFile(compiler));

  browser.on('bundle', bundle => bundle.on('end', () => compiler.close()));

  browser.on('reset', () => setupPipeline(browser, compiler.options));
}

export default browserifyKnot;
