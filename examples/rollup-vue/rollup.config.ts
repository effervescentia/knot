import knotPlugin from '@knot/rollup-plugin';
import commonjs from '@rollup/plugin-commonjs';
import resolve from '@rollup/plugin-node-resolve';
import nodeGlobals from 'rollup-plugin-node-globals';

export default args => ({
  input: 'src/index.js',
  output: {
    file: 'bundle.js',
    format: 'iife'
  },
  plugins: [
    knotPlugin({
      // WARNING: providing an override path for the knot compiler is not recommended
      knot: args.configKnotc,
      plugins: {
        jsx: '@knot/vue-plugin'
      }
    }),
    resolve(),
    commonjs({
      exclude: ['node_modules/symbol-observable/es/*.js']
    }),
    nodeGlobals()
  ]
});
