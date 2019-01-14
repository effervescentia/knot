const HtmlWebpackPlugin = require('html-webpack-plugin');

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
    new HtmlWebpackPlugin({
      template: 'index.html'
    })
  ],

  module: {
    rules: [{
      test: /\.kn$/,
      loader: '@knot/webpack-loader'
    }]
  },

  devServer: {
    historyApiFallback: true,
    open: true,
    port: 1337
  }
};