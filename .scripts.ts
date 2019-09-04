// tslint:disable: no-reference object-literal-sort-keys
/// <reference path="./scripts/nps-utils.d.ts" />

import { concurrent, series } from 'nps-utils';
import { DEFAULT_OPTIONS } from './scripts';

const PKG_FILTER = '--ignore=@knot/*-example';
const EXAMPLE_FILTER = '--scope=@knot/*-example';

const run = (args: string) => `lerna run ${args}`;
const pkgRun = (task: string) => run(`${PKG_FILTER} ${task}`);
const exampleRun = (task: string) => run(`${EXAMPLE_FILTER} ${task}`);

export default {
  options: DEFAULT_OPTIONS,

  scripts: {
    setup: {
      description: 'bootstrap lerna project',
      script: 'lerna bootstrap'
    },

    build: {
      default: {
        description: 'build all projects in repo',
        script: concurrent.nps('build.packages', 'build.compiler')
      },

      packages: {
        description: 'build all packages in repo',
        script: pkgRun('build')
      },
      compiler: {
        description: 'build compiler',
        script: '(cd compiler && esy release)'
      },
    },

    test: {
      default: {
        description: 'test all projects in repo',
        script: series.nps('test.packages', 'test.examples')
      },

      packages: {
        description: 'test all packages in repo',
        script: pkgRun('test')
      },
      examples: {
        description: 'test all examples in repo',
        script: exampleRun('test')
      }
    },

    release: {
      description:
        'release all packages in repo, auto version and update interdependencies',
      script: 'iolaus --repository="https://github.com/effervescentia/knot"'
    }
  }
};
