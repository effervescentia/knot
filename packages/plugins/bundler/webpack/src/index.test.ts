import { Options, Target } from '@knot/compiler';
import test from 'ava';

import KnotCompilerPlugin from '.';

test('construct', (t) => {
  const options = { debug: true, target: Target.JAVASCRIPT_COMMON } as Options;

  const plugin = new KnotCompilerPlugin(options);

  t.deepEqual(plugin.options, options);
});
