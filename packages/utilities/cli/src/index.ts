import chalk from 'chalk';
import { Command } from 'commander';
import gulp from 'gulp';
import conflict from 'gulp-conflict';
import rename from 'gulp-rename';
import template from 'gulp-template';
import inquirer from 'inquirer';
import latestVersion from 'latest-version';
import path from 'path';

const INTERPOLATION_PATTERN = /#{([\S\s]+?)}/g;

const program = new Command('knot');

enum BundlerType {
  WEBPACK = 'webpack',
  ROLLUP = 'rollup',
  BROWSERIFY = 'browserify'
}

enum FrameworkType {
  REACT = 'react',
  VUE = 'vue'
}

const FRAMEWORKS = [FrameworkType.REACT, FrameworkType.VUE];
const FRAMEWORK_BUNDLERS = {
  [FrameworkType.REACT]: [
    BundlerType.WEBPACK,
    BundlerType.BROWSERIFY,
    BundlerType.ROLLUP
  ],
  [FrameworkType.VUE]: [BundlerType.WEBPACK, BundlerType.ROLLUP]
};

program.version('1.0.0');

program
  .command('init [target_dir]')
  .description('Setup a new knot project')
  .action(async (targetDir = '.') => {
    const { frameworkType } = await inquirer.prompt<{
      readonly frameworkType: FrameworkType;
    }>({
      choices: FRAMEWORKS,
      message: 'Which rendering library do you want to use?',
      name: 'frameworkType',
      type: 'list'
    });

    const { bundlerType } = await inquirer.prompt<{
      readonly bundlerType: BundlerType;
    }>({
      choices: FRAMEWORK_BUNDLERS[frameworkType],
      message: 'Which bundler do you want to use?',
      name: 'bundlerType',
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
      type: 'input',
      filter: value => value.trim(),
      validate: value =>
        value.trim().length !== 0 || 'Project name must not be empty'
    });

    console.log(
      `${chalk.blue(
        '!'
      )} Thank you! Generating files for a knot project (${projectName}) with "${frameworkType} + ${bundlerType}" now...`
    );

    const templatesDir = path.resolve(__dirname, '../..', 'templates');
    const commonDir = path.resolve(templatesDir, 'common');
    const httpsDir = path.resolve(templatesDir, 'https');
    const frameworkDir = path.resolve(templatesDir, frameworkType);
    const bundlerDir = path.resolve(templatesDir, bundlerType);

    const reactPluginVersion = await latestVersion('@knot/react-plugin');
    const vuePluginVersion = await latestVersion('@knot/vue-plugin');
    const webpackPluginVersion = await latestVersion('@knot/webpack-plugin');
    const rollupPluginVersion = await latestVersion('@knot/rollup-plugin');
    const browserifyPluginVersion = await latestVersion(
      '@knot/browserify-plugin'
    );

    gulp
      .src(
        [
          commonDir,
          frameworkDir,
          bundlerDir,
          ...(isHTTPS ? [httpsDir] : [])
        ].map(dir => `${dir}/**`)
      )
      .pipe(
        rename(file => {
          if (file.basename.startsWith('_')) {
            file.basename = file.basename.slice(1);
          }
        })
      )
      .pipe(
        template(
          {
            isHTTPS,
            projectName,
            bundlerType,
            frameworkType,
            pluginVersions: {
              react: reactPluginVersion,
              vue: vuePluginVersion,
              browserify: browserifyPluginVersion,
              webpack: webpackPluginVersion,
              rollup: rollupPluginVersion
            }
          },
          { interpolate: INTERPOLATION_PATTERN }
        )
      )
      .pipe(conflict(targetDir))
      .pipe(gulp.dest(targetDir));
  });

program.parse(process.argv);

if (!program.args.length) {
  program.help();
}
