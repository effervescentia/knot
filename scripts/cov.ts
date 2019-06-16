// tslint:disable object-literal-sort-keys
import { open, series } from 'nps-utils';

const nycReport = (reporter: 'html' | 'lcov') =>
  `nyc report --reporter=${reporter}`;

export default {
  default: {
    description:
      'build project, run unit tests and generate coverage information',
    script: series(
      series.nps('build', 'test.unit', 'cov.html'),
      open('coverage/index.html')
    )
  },

  html: {
    description: 'generate interactive web coverage report',
    script: nycReport('html')
  },
  send: {
    description:
      'generate LCOV formatted coverage report and upload to codecov',
    script: series(`${nycReport('lcov')} > coverage.lcov`, 'codecov')
  },
  check: {
    description: 'test report for 100% coverage',
    script: series(
      'nyc report',
      'nyc check-coverage --lines 100 --functions 100 --branches 100'
    )
  }
};
