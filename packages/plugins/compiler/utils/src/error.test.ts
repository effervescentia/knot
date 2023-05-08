import test from 'ava';

import PluginError from './error';

test('PluginError extends error', (t) =>
  t.true(new PluginError('failed') instanceof Error));
