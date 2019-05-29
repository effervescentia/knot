import KnotPlugin from '@knot/webpack-plugin';
import * as HtmlPlugin from 'html-webpack-plugin';

export default {
  entry: './src/index.js',
  output: {
    filename: 'app.js',
    path: __dirname + '/dist'
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
    port: 1337
  }
};
