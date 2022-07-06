import inquirer from 'inquirer';

import {
  BundlerType,
  FRAMEWORK_BUNDLERS,
  FRAMEWORKS,
  FrameworkType,
} from './contants';

export const frameworkPrompt = () =>
  inquirer.prompt<{
    readonly frameworkType: FrameworkType;
  }>({
    choices: FRAMEWORKS,
    message: 'Which rendering library do you want to use?',
    name: 'frameworkType',
    type: 'list',
  });

export const bundlerPrompt = (frameworkType: FrameworkType) =>
  inquirer.prompt<{
    readonly bundlerType: BundlerType;
  }>({
    choices: FRAMEWORK_BUNDLERS[frameworkType],
    message: 'Which bundler do you want to use?',
    name: 'bundlerType',
    type: 'list',
  });

export const httpsPrompt = () =>
  inquirer.prompt<{
    readonly isHTTPS: boolean;
  }>({
    default: true,
    message: 'Do you want to use HTTPS when running locally?',
    name: 'isHTTPS',
    type: 'confirm',
  });

export const projectNamePrompt = () =>
  inquirer.prompt<{
    readonly projectName: string;
  }>({
    message: 'What is the name of your project?',
    name: 'projectName',
    type: 'input',
    filter: (value) => value.trim(),
    validate: (value) =>
      value.trim().length !== 0 || 'Project name must not be empty',
  });
