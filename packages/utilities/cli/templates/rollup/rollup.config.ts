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
    knotPlugin(),
    resolve(),
    commonjs({
      namedExports: {
        react: ['createElement', 'Component', 'Fragment'],
        'react-dom': ['render']
      }
    }),
    nodeGlobals()
  ]
});
