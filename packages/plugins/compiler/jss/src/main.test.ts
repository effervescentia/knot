import test from 'ava';

import main from './main';

test('includes keys', (t) => {
  const keys = Object.keys(main);

  t.plan(keys.length);

  ['resolve', 'classes', 'styleExpressionPlugin', 'styleRulePlugin'].forEach(
    (key) => t.true(keys.includes(key))
  );
});
