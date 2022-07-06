import { JSXPlugin } from '@knot/plugin-utils';
import test from 'ava';
import mock from 'mock-require';

mock('@knot/platform', {});
mock('@knot/jsx', {});
mock('@knot/style', {});

import knot from '.';

test('utils', (t) => {
  t.deepEqual(knot.platform, {});
  t.deepEqual(knot.jsx, {} as JSXPlugin<unknown, unknown>);
  t.deepEqual(knot.style, {});
});
