// tslint:disable:no-expression-statement
import test from 'ava';
import * as React from 'react';
import { main } from '.';

test('includes keys', t => {
  const keys = Object.keys(main);

  t.plan(keys.length);

  ['createElement', 'render'].forEach(key => t.true(keys.includes(key)));
});

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
