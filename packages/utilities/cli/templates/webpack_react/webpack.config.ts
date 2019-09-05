import KnotPlugin from '@knot/webpack-plugin';
import * as fs from 'fs';
import * as path from 'path';
import * as HtmlPlugin from 'html-webpack-plugin';

const BUILD_DIR = path.join(__dirname, 'dist');
const CERTS_DIR = path.join(__dirname, 'certs');

export default {
  entry: './src/index.js',
  output: {
    filename: 'app.js',
    path: BUILD_DIR
  },

  resolve: {
    extensions: ['.kn', '.js', '.json']
  },

  plugins: [
    new HtmlPlugin({
      template: 'index.html'
    }),
    new KnotPlugin({})
  ],

  devServer: {
    historyApiFallback: true,
    open: true,
    port: 1337,

    https: {
      key: fs.readFileSync(path.join(CERTS_DIR, 'localhost.key')),
      cert: fs.readFileSync(path.join(CERTS_DIR, 'localhost.crt'))
    }
  }
};
