import test from 'ava';

import { prop } from '.';

test('prop()', t => {
  const myValue = 123;
  const options = { myKey: myValue };

  t.is(prop(options, 'myKey'), myValue);
  t.is(prop(options, 'otherKey'), undefined);
  t.is(prop(options, 'otherKey', 987), 987);
});
