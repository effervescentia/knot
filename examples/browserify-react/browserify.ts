// tslint:disable: no-expression-statement
import knotify from '@knot/browserify-plugin';
import * as browserify from 'browserify';
import * as fs from 'fs';
import * as path from 'path';
import { argv } from 'yargs';

browserify('src/index.js')
  .plugin(knotify, {
    // debug: true,
    // WARNING: providing an override path for the knot compiler is not recommended
    knot: argv.knotc as string
  })
  .bundle()
  // tslint:disable-next-line: no-console
  .on('error', error => console.error(error.toString()))
  .pipe(fs.createWriteStream(path.join(__dirname, 'bundle.js')));
