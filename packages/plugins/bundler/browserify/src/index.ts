import KnotCompiler, { OptionOverrides, Target } from '@knot/compiler';
import { BrowserifyObject } from 'browserify';

import setupPipeline from './setup';
import transformFile from './transform';

function browserifyKnot(
  browser: BrowserifyObject,
  options: OptionOverrides = {}
): void {
  const configuredOptions: OptionOverrides = {
    ...options,
    target: Target.JAVASCRIPT_COMMON,
  };

  const compiler = new KnotCompiler(configuredOptions);

  setupPipeline(browser, compiler.options);

  browser.transform(transformFile(compiler));

  browser.on('bundle', (bundle) => bundle.on('end', () => compiler.close()));

  browser.on('reset', () => setupPipeline(browser, compiler.options));
}

export default browserifyKnot;
