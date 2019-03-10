// tslint:disable:no-expression-statement
import test from 'ava';
import { main } from '.';

test('includes keys', t => {
  const keys = Object.keys(main);

  t.plan(keys.length);

  ['createElement', 'render'].forEach(key => t.true(keys.includes(key)));
});
