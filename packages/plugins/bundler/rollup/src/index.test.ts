import test from 'ava';

import knotRollupPlugin from '.';

test('knotRollupPlugin()', t => {
  const plugin = knotRollupPlugin();

  t.is(plugin.name, 'knot');
  t.is(typeof plugin.buildStart, 'function');
  t.is(typeof plugin.resolveId, 'function');
  t.is(typeof plugin.transform, 'function');
  t.is(typeof plugin.buildEnd, 'function');
});
