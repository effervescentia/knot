import knotPlugin from '@knot/rollup-plugin';
import commonjs from '@rollup/plugin-commonjs';
import resolve from '@rollup/plugin-node-resolve';
import * as path from 'path';

export default commandLineArgs => ({
  input: 'src/index.js',
  output: {
    file: 'bundle.js',
    format: 'iife'
  },
  plugins: [
    knotPlugin({
      // WARNING: providing an override path for the knot compiler is not recommended
      knot: commandLineArgs.configKnotc
    }),
    resolve({
      modulesOnly: true
    }),
    commonjs()
  ]
});
