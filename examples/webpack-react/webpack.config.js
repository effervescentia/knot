const HtmlPlugin = require('html-webpack-plugin');
const KnotPlugin = require('@knot/webpack-plugin');

module.exports = {
  entry: './src/index.js',
  output: {
    path: __dirname + '/dist',
    filename: 'app.js'
  },

  resolve: {
    extensions: ['.kn', '.js', '.json']
  },

  plugins: [
    new HtmlPlugin({
      template: 'index.html'
    }),
    new KnotPlugin({
      knot: process.env.KNOT_BINARY
    })
  ],

  devServer: {
    historyApiFallback: true,
    open: true,
    port: 1337
  }
};