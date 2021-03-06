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
      plugins: {
        jsx: '@knot/#{frameworkType}-plugin'
      }
    }),
    resolve(),
    commonjs({<% if (frameworkType === 'react') { %>
      namedExports: {
        react: ['createElement', 'Component', 'Fragment'],
        'react-dom': ['render']
      } <% } else if (frameworkType === 'vue') { %>
      exclude: ['node_modules/symbol-observable/es/*.js']<% } %>
    }),
    nodeGlobals()
  ]
});
