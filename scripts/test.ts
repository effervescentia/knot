import { series } from 'nps-utils';

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
      series.nps('build', 'test.lint.ci', 'test.unit.ci', 'cov.lcov')
    )
  },
  lint: {
    default: {
      description: 'run code linters',
      script: 'eslint --quiet "src/**/*.ts"'
    },

    ci: series(
      'mkdir -p reports/eslint',
      'test.lint --format junit > reports/eslint/report.xml'
    )
  },
  unit: {
    default: {
      description: 'run unit tests',
      script: 'nyc --silent ava'
    },

    ci: series(
      'mkdir -p reports/ava',
      'nps "test.unit --tap | tap-xunit > reports/ava/report.xml"'
    )
  }
};
