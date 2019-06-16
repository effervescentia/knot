import { series } from 'nps-utils';

export default {
  default: {
    description: 'run all file formatters',
    script: series.nps('fix.prettier', 'fix.tslint')
  },

  prettier: {
    description: 'run prettier formatter on typescript files',
    script: 'prettier "src/**/*.ts" --write'
  },
  tslint: {
    description: 'run tslint formatter on typescript files',
    script: 'tslint --fix --project .'
  }
};
