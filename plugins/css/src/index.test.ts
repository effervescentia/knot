// tslint:disable:no-expression-statement
import test from 'ava';
import { main } from '.';

test('includes keys', t => {
  const keys = Object.keys(main);

  t.plan(keys.length);

  [
    'fontSize',
    'backgroundColor',
    'visibility',
    'display',
    'hidden',
    'flex',
    'red',
    'px'
  ].forEach(key => t.true(keys.includes(key)));
});
