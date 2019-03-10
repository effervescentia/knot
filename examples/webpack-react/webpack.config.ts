import * as HtmlPlugin from 'html-webpack-plugin';
import KnotPlugin from '@knot/webpack-plugin';

export default {
  entry: './src/index.js',
  output: {
    path: __dirname + '/dist',
    filename: 'app.js',
  },

  resolve: {
    extensions: ['.kn', '.js', '.json'],
  },

  plugins: [
    new HtmlPlugin({
      template: 'index.html',
    }),
    new KnotPlugin({
      knot: process.env.KNOT_BINARY,
      debug: true,
    }),
  ],

  devServer: {
    historyApiFallback: true,
    open: true,
    port: 1337,
  },
};
