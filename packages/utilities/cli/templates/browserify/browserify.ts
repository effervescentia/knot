import knotify from '@knot/browserify-plugin';
import * as browserify from 'browserify';
import * as fs from 'fs';
import * as path from 'path';

browserify('src/index.js')
  .plugin(knotify)
  .bundle()
  .on('error', error => console.error(error.toString()))
  .pipe(fs.createWriteStream(path.join(__dirname, 'bundle.js')));
