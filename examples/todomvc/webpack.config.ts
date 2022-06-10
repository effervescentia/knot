import KnotPlugin from '@knot/webpack-plugin';
import * as HtmlPlugin from 'html-webpack-plugin';
import * as path from 'path';

export default env => ({
  entry: './src/index.js',

  output: {
    filename: 'app.js',
    path: path.resolve(__dirname, 'dist')
  },

  resolve: {
    extensions: ['.kn', '.js', '.json']
  },

  plugins: [
    new HtmlPlugin({
      template: 'index.html'
    }),
    new KnotPlugin({
      // debug: true,
      knotc: env && env.knotc
    })
  ],

  devServer: {
    historyApiFallback: true,
    open: true,
    port: 1337
  }
});
