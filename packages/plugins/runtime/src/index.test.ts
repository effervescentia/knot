import test from 'ava';
import mock from 'mock-require';

mock('@knot/platform', {});
mock('@knot/jsx', {});
mock('@knot/style', {});

import knot from '.';

test('utils', t => {
  t.deepEqual(Object.keys(knot), ['jsx', 'platform', 'style']);
});
