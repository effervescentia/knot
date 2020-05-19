import { isMultisetEqual } from '@knot/common';
import test, { ExecutionContext } from 'ava';

import { main } from '.';

function hasKeys<T extends object>(
  t: ExecutionContext,
  actual: T,
  expected: (keyof T | string)[]
): void {
  const keys = Object.keys(actual);

  t.true(
    isMultisetEqual(keys, expected),
    `${keys} did not contain the same members as ${expected}`
  );
}

test('includes keys', t =>
  hasKeys(t, main, [
    'createComponent',
    'createElement',
    'createFragment',
    'render',
    'withState'
  ]));
