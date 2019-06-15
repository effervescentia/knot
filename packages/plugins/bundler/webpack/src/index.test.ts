// tslint:disable:no-expression-statement no-object-literal-type-assertion
import test from 'ava';
import KnotCompilerPlugin, { DEFAULT_OPTIONS } from '.';
import { Options } from './types';

test('construct', t => {
  const options = { port: 90210 } as Options;

  const plugin = new KnotCompilerPlugin(options);

  t.deepEqual(plugin.options, { ...DEFAULT_OPTIONS, ...options });
});
