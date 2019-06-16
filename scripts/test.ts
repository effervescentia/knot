import { series } from 'nps-utils';

export default {
  default: {
    description: 'build project then run code linters and unit tests',
    script: series.nps('build', 'test.lint', 'test.unit')
  },

  lint: {
    description: 'run code linters',
    script: series(
      'tslint --project .',
      'prettier "src/**/*.ts" --list-different'
    )
  },
  unit: {
    description: 'run unit tests',
    script: 'nyc --silent ava'
  }
};
