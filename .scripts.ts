/// <reference path="./scripts/nps-utils.d.ts" />

import { concurrent, series } from 'nps-utils';
import path from 'path';

import { DEFAULT_OPTIONS } from './scripts';

const PKG_FILTER = '--ignore=@knot/*-example';
const EXAMPLE_FILTER = '--scope=@knot/*-example';
const LOCAL_BINARY = path.join(
  __dirname,
  'compiler/_esy/default/build/install/default/bin/knotc.exe'
);

const run = (args: string, filter?: string) =>
  `lerna run ${filter ? `--scope=${filter} ` : ''}${args}`;
const pkgRun = (task: string) => run(`${PKG_FILTER} ${task}`);
const exampleRun = (task: string) => run(`${EXAMPLE_FILTER} ${task}`);

const webpackScript = (script: string, framework: string) => ({
  description: `run the "webpack + ${framework}" example`,
  script: run(
    `${script} -- --env.knotc='${LOCAL_BINARY}'`,
    `@knot/webpack-${framework}-example`
  )
});
const webpackExample = (framework: string) => ({
  default: webpackScript('start', framework),
  debug: webpackScript('start:debug', framework)
});

const browserifyExample = (framework: string) => ({
  default: {
    description: `run the "browserify + ${framework}" example`,
    script: series.nps(
      `start.example.browserify_${framework}.build`,
      `start.example.browserify_${framework}.serve`
    )
  },

  build: {
    description: `build the "browserify + ${framework}" example`,
    script: run(
      `build -- --knotc='${LOCAL_BINARY}'`,
      `@knot/browserify-${framework}-example`
    )
  },
  serve: {
    description: `serve the "browserify + ${framework}" example`,
    script: run('start', `@knot/browserify-${framework}-example`)
  }
});

const rollupExample = (framework: string) => ({
  default: {
    description: `run the "rollup + ${framework}" example`,
    script: series.nps(
      `start.example.rollup_${framework}.build`,
      `start.example.rollup_${framework}.serve`
    )
  },

  build: {
    description: `build the "rollup + ${framework}" example`,
    script: run(
      `build -- --configKnotc='${LOCAL_BINARY}'`,
      `@knot/rollup-${framework}-example`
    )
  },
  serve: {
    description: `serve the "rollup + ${framework}" example`,
    script: run('start', `@knot/rollup-${framework}-example`)
  }
});

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
        webpack_react: webpackExample('react'),
        webpack_vue: webpackExample('vue'),
        browserify_react: browserifyExample('react'),
        browserify_vue: browserifyExample('vue'),
        rollup_react: rollupExample('react'),
        rollup_vue: rollupExample('vue'),
        todo: {
          description: 'run the "todo" example',
          script: run(
            `start -- --env.knotc='${LOCAL_BINARY}'`,
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
