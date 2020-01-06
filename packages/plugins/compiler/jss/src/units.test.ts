import test from 'ava';

import units, { unit } from './units';

test('includes keys', t => {
  const keys = Object.keys(units);

  t.plan(keys.length);

  [
    'ch',
    'cm',
    'deg',
    'dpcm',
    'dpi',
    'dppx',
    'em',
    'ex',
    'fr',
    'Hz',
    'in',
    'kHz',
    'mm',
    'ms',
    'pc',
    'percent',
    'pt',
    'px',
    'rem',
    's',
    'vh',
    'vw',
    'x'
  ].forEach(key => t.true(keys.includes(key)));
});

test('adds appropriate suffix', t => {
  const keys = Object.keys(units);

  keys.forEach(key => {
    const value = Math.round(Math.random() * 20);

    // eslint-disable-next-line sonarjs/no-small-switch
    switch (key) {
      case 'percent':
        t.is(units[key](value), `${value}%`);
        return;
      default:
        t.is(units[key](value), `${value}${key}`);
    }
  });
});

test('unit()', t => {
  t.is('123abc', unit('abc')(123));
});
