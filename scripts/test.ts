import { series } from 'nps-utils';

export default {
  default: {
    description: 'build project then run code linters and unit tests',
    script: series.nps('build', 'test.lint', 'test.unit')
  },

  ci: {
    description:
      'build project then run code linters and unit tests and upload coverage',
    script: series.nps('test', 'cov.lcov')
  },
  lint: {
    description: 'run code linters',
    script: series(
      'eslint --quiet "src/**/*.ts"',
      'prettier "src/**/*.ts" --list-different'
    )
  },
  unit: {
    description: 'run unit tests',
    script: 'nyc --silent ava'
  }
};
