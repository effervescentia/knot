import chalk from 'chalk';
import { Command } from 'commander';

import pkg from '../package.json';
import {
  bundlerPrompt,
  frameworkPrompt,
  httpsPrompt,
  projectNamePrompt
} from './prompts';
import { scaffoldProject } from './scaffold';

const program = new Command('knot');

program.version(pkg.version);

program
  .command('init [target_dir]')
  .description('Setup a new knot project')
  .action(async (targetDir = '.') => {
    const { frameworkType } = await frameworkPrompt();
    const { bundlerType } = await bundlerPrompt(frameworkType);
    const { isHTTPS } = await httpsPrompt();
    const { projectName } = await projectNamePrompt();

    console.log(
      `${chalk.blue(
        '!'
      )} Thank you! Generating files for a knot project (${projectName}) with "${frameworkType} + ${bundlerType}" now...`
    );

    scaffoldProject({
      isHTTPS,
      projectName,
      frameworkType,
      bundlerType,
      targetDir
    });
  });

program.parse(process.argv);

if (!program.args.length) {
  program.help();
}
