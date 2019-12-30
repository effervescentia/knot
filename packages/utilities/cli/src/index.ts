// tslint:disable:no-expression-statement
import chalk from 'chalk';
import { Command } from 'commander';
import gulp from 'gulp';
import conflict from 'gulp-conflict';
import filter from 'gulp-filter';
import rename from 'gulp-rename';
import template from 'gulp-template';
import inquirer from 'inquirer';
import latestVersion from 'latest-version';
import path from 'path';

const program = new Command('knot');

enum ProjectType {
  WEBPACK_REACT = 'webpack + react'
}

const PROJECT_DIRS: Record<ProjectType, string> = {
  [ProjectType.WEBPACK_REACT]: 'webpack_react'
};

program.version('1.0.0');

program
  .command('init [target_dir]')
  .description('Setup a new knot project')
  .action(async (targetDir = '.') => {
    const { projectType } = await inquirer.prompt<{
      readonly projectType: string;
    }>({
      choices: [ProjectType.WEBPACK_REACT],
      message: 'What type of project do you want to create?',
      name: 'projectType',
      type: 'list'
    });

    const { isHTTPS } = await inquirer.prompt<{
      readonly isHTTPS: boolean;
    }>({
      default: true,
      message: 'Do you want to use HTTPS when running locally?',
      name: 'isHTTPS',
      type: 'confirm'
    });

    const { projectName } = await inquirer.prompt<{
      readonly projectName: string;
    }>({
      message: 'What is the name of your project?',
      name: 'projectName',
      type: 'input'
    });

    // tslint:disable-next-line: no-console
    console.log(
      `${chalk.blue(
        '!'
      )} Thank you! Generating files for a knot project (${projectName}) with "${projectType}" now...`
    );

    const sourceDir = path.resolve(
      __dirname,
      '../..',
      'templates',
      PROJECT_DIRS[projectType]
    );

    const webpackPluginVersion = await latestVersion('@knot/webpack-plugin');

    gulp
      .src(`${sourceDir}/**`)
      .pipe(filter(file => isHTTPS || file.basename !== 'certs'))
      .pipe(
        rename(file => {
          if (file.basename.startsWith('_')) {
            // tslint:disable-next-line: no-object-mutation
            file.basename = file.basename.slice(1);
          }
        })
      )
      .pipe(
        template(
          {
            isHTTPS,
            projectName,
            webpackPluginVersion
          },
          { interpolate: /#{([\s\S]+?)}/g }
        )
      )
      .pipe(conflict(targetDir))
      .pipe(gulp.dest(targetDir));
  });

program.parse(process.argv);

if (!program.args.length) {
  program.help();
}
