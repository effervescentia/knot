import { concurrent, rimraf, series } from 'nps-utils';

export const watch = {
  description:
    'watch for changes in source files, rebuild and run tests when a change is detected',
  script: series(
    'nps build',
    `${concurrent.nps('build.main -- -w', 'test.unit -- --watch')}`
  )
};

export const reset = {
  description:
    'remove all untracked files, undo all uncommitted changes and install dependencies',
  script: series('git clean -dfx', 'git reset --hard', 'yarn install')
};

export const clean = {
  description: 'remove generated directories',
  script: rimraf('build test')
};

export const all = {
  description: 'reset project, run tests and generate documentation',
  script: series.nps('reset', 'test', 'cov.check', 'doc.html')
};

export const preprelease = {
  description:
    'reset project, run tests, update the version and publish the latest documentation',
  script: series.nps('all', 'version', 'doc.publish')
};
