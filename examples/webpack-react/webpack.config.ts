import KnotPlugin from '@knot/webpack-plugin';
import * as HtmlPlugin from 'html-webpack-plugin';
// import { BundleAnalyzerPlugin } from 'webpack-bundle-analyzer';

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
      // debug: true,
    }),
    // new BundleAnalyzerPlugin(),
  ],

  devServer: {
    historyApiFallback: true,
    open: true,
    port: 1337,
  },
};
