import test, { ExecutionContext } from 'ava';

import * as Common from '.';
import { isMultisetEqual } from './utils';

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

test('includes keys', (t) =>
  hasKeys(t, Common, [
    'noop',
    'isMultisetEqual',
    'addEntry',
    'fromEntries',
    'mapEntries',
    'mapKeys',
    'mapValues',
    'filterEntries',
    'filterKeys',
    'filterValues',
    'groupReduce',
    'groupValues',
    'groupEntries',
  ]));
