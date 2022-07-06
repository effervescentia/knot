import test from 'ava';

import { isKnot } from './utils';

test('isKnot()', (t) => {
  t.true(isKnot('file.kn'));
  t.true(isKnot('file.knot'));
  t.false(isKnot('file.k'));
  t.false(isKnot('file.js'));
});
