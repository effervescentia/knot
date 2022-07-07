import test from 'ava';
import path from 'path';

import * as Compiler from '.';

test('compiler binary path', (t) => {
  t.is(Compiler.path, path.resolve(process.cwd(), 'bin', 'knotc.exe'));
});
