import KnotPlugin from '@knot/webpack-plugin';
import * as HtmlPlugin from 'html-webpack-plugin';
import * as path from 'path';
// import { BundleAnalyzerPlugin } from 'webpack-bundle-analyzer';

export default {
  entry: './src/index.js',

  output: {
    filename: 'app.js',
    path: path.resolve(__dirname, 'dist'),
  },

  resolve: {
    extensions: ['.kn', '.js', '.json'],
  },

  plugins: [
    new HtmlPlugin({
      template: 'index.html',
    }),
    new KnotPlugin({
      // debug: true,
      knot: process.env.KNOT_BINARY,
    }),
    // new BundleAnalyzerPlugin(),
  ],

  devServer: {
    historyApiFallback: true,
    open: true,
    port: 1337,
  },
};
