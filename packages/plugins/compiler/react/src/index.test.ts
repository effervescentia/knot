import test, { ExecutionContext } from 'ava';
import * as React from 'react';

import { main } from '.';

function hasKeys<T extends object>(
  t: ExecutionContext,
  actual: T,
  expected: ReadonlyArray<keyof T | string>
): void {
  const keys = Object.keys(actual);

  keys.forEach(key => t.true(expected.includes(key as any)));
  t.is(keys.length, expected.length);
}

test('includes keys', t =>
  hasKeys(t, main, ['createElement', 'createFragment', 'render', 'withState']));

test('createElement()', t => {
  t.is(main.createElement, React.createElement);
});

test('render()', t => {
  const elementId = 'myApp';

  const rootEl = document.createElement('div');
  rootEl.setAttribute('id', elementId);
  document.body.appendChild(rootEl);

  main.render(React.createElement('h1', null, 'Hello, World!'), elementId);

  t.is(document.getElementById(elementId).innerHTML, '<h1>Hello, World!</h1>');
});

test('withState()', t => {
  const Component = main.withState(() => null, () => null);
  const instance = new Component({});

  t.true(instance instanceof React.Component);
  hasKeys(t, instance, ['_state', 'props', 'context', 'refs', 'updater']);
});
