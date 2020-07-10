import { concurrent, series } from 'nps-utils';

const eslintCommand = 'eslint --quiet "src/**/*.ts"';

export default {
  default: {
    description: 'build project then run code linters and unit tests',
    script: series.nps('build', 'test.lint', 'test.unit')
  },

  ci: {
    description:
      'build project then run code linters and unit tests with coverage',
    script: series(
      'mkdir -p reports',
      series.nps(
        'build',
        'test.lint.ci',
        'test.unit --tap | tap-xunit > reports/ava.xml',
        'cov.lcov'
      )
    )
  },
  lint: {
    default: {
      description: 'run code linters',
      script: concurrent.nps('test.lint.eslint', 'test.lint.prettier')
    },

    ci: concurrent.nps(
      'test.lint.eslint --format junit > reports/eslint.xml',
      'test.lint.prettier'
    ),
    eslint: {
      description: 'run eslint',
      script: eslintCommand
    },
    prettier: {
      description: 'run prettier',
      script: 'prettier "src/**/*.ts" --list-different'
    }
  },
  unit: {
    description: 'run unit tests',
    script: 'nyc --silent ava'
  }
};
