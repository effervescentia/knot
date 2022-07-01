import test from 'ava';

import colors from './colors';
import main from './main';
import properties from './properties';
import units from './units';

test('includes keys', (t) => {
  const keys = Object.keys(main);

  t.plan(keys.length);

  [
    ...Object.keys(properties),
    ...Object.keys(colors),
    ...Object.keys(units),

    'visibility',
    'hidden',
    'none',

    'num',
    'str',

    'resolve',
    'classes',
  ].forEach((key) => t.true(keys.includes(key)));
});
