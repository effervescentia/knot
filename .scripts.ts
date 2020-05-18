/// <reference path="./scripts/nps-utils.d.ts" />

import { concurrent, series } from 'nps-utils';
import { DEFAULT_OPTIONS } from './scripts';

const PKG_FILTER = '--ignore=@knot/*-example';
const EXAMPLE_FILTER = '--scope=@knot/*-example';

const run = (args: string, filter?: string) =>
  `lerna run ${filter ? `--scope=${filter} ` : ''}${args}`;
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
      utils: {
        description: 'build plugin utils',
        script: run('build', '@knot/plugin-utils')
      },
      react: {
        description: 'build react plugin',
        script: run('build', '@knot/react-plugin')
      },
      vue: {
        description: 'build vue plugin',
        script: run('build', '@knot/vue-plugin')
      }
    },

    start: {
      example: {
        webpack_react: {
          description: 'run the "webpack + react" example',
          script: run(
            "start -- --env.knotc='esy x -P ../../compiler knotc.exe'",
            '@knot/webpack-react-example'
          )
        },
        webpack_vue: {
          description: 'run the "webpack + vue" example',
          script: run(
            "start -- --env.knotc='esy x -P ../../compiler knotc.exe'",
            '@knot/webpack-vue-example'
          )
        },
        browserify_react: {
          default: {
            description: 'run the "browserify + react" example',
            script: series.nps(
              'start.example.browserify_react.build',
              'start.example.browserify_react.serve'
            )
          },

          build: {
            description: 'build the "browserify + react" example',
            script: run(
              "build -- --knotc='esy x -P ../../compiler knotc.exe'",
              '@knot/browserify-react-example'
            )
          },
          serve: {
            description: 'serve the "browserify + react" example',
            script: run('start', '@knot/browserify-react-example')
          }
        },
        rollup_react: {
          default: {
            description: 'run the "rollup + react" example',
            script: series.nps(
              'start.example.rollup_react.build',
              'start.example.rollup_react.serve'
            )
          },

          build: {
            description: 'build the "rollup + react" example',
            script: run(
              "build -- --configKnotc='esy x -P ../../compiler knotc.exe'",
              '@knot/rollup-react-example'
            )
          },
          serve: {
            description: 'serve the "rollup + react" example',
            script: run('start', '@knot/rollup-react-example')
          }
        },
        todo: {
          description: 'run the "todo" example',
          script: run(
            "start -- --env.knotc='esy x -P ../../compiler knotc.exe'",
            '@knot/todomvc-example'
          )
        }
      }
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
