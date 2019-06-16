import { open, series } from 'nps-utils';

const TYPEDOC = 'typedoc src/ --exclude **/*.spec.ts --target ES6 --mode file';

export default {
  default: {
    description: 'generate and view documentation',
    script: series('nps doc.html', open('build/docs/index.html'))
  },

  html: {
    description: 'generate interactive web coverage report',
    script: `${TYPEDOC} --out build/docs`
  },
  json: {
    description: 'generate JSON file describing types in project',
    script: `${TYPEDOC} --json build/docs/typedoc.json`
  },
  publish: {
    description: 'publish documentation to github pages',
    script: 'gh-pages -m "[ci skip] Updates" -d build/docs'
  }
};
