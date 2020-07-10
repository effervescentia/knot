import test from 'ava';

import pkg from '../package.json';
import { cli } from './_suite';

test('--version', async t => {
  const { stdout } = await cli('--version');

  t.is(stdout, pkg.version);
});

test('--help', async t => {
  const { stdout } = await cli('--help');

  t.is(
    stdout,
    `Usage: knot [options] [command]

Options:
  -V, --version      output the version number
  -h, --help         output usage information

Commands:
  init [target_dir]  Setup a new knot project`
  );
});
