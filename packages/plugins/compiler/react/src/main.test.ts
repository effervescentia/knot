import { isMultisetEqual } from '@knot/common';
import test, { ExecutionContext } from 'ava';
import * as React from 'react';

import main from './main';

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
  hasKeys(t, main, [
    // 'createComponent',
    'createTag',
    'createFragment',
    'render',
    'bindStyle',
    // 'withState',
  ]));

test('createTag()', (t) => {
  t.is(main.createTag, React.createElement);
});

test('render()', (t) => {
  const elementId = 'myApp';

  const rootEl = document.createElement('div');
  rootEl.setAttribute('id', elementId);
  document.body.appendChild(rootEl);

  main.render(React.createElement('h1', null, 'Hello, World!'), elementId);

  t.is(document.getElementById(elementId).innerHTML, '<h1>Hello, World!</h1>');
});

// test('withState()', (t) => {
//   const Component = main.withState(
//     () => ({ get: () => null }),
//     () => null
//   ) as any;
//   const instance = new Component({});

//   t.true(instance instanceof React.Component);
//   hasKeys(t, instance, ['_state', 'props', 'context', 'refs', 'updater']);
// });
