const {
  getOptions
} = require('loader-utils');

const GLOBALS = '$$knot_globals$$'
const DEFAULT_OPTIONS = {
  plugins: {
    jsx: '@knot/react-plugin',
    style: '@knot/css-plugin',
    utils: '@knot/javascript-utils'
  }
};

module.exports = function loader(source) {
  const callback = this.async();
  const options = {
    ...DEFAULT_OPTIONS,
    ...getOptions(this)
  };

  options.compiler.generate(source)
    .then((result) => {
      const wrapped = 'module.exports=(' + result + ')({' + //
        '"' + GLOBALS + '":{' + //
        'jsx:require("' + options.plugins.jsx + '"),' + //
        'style:require("' + options.plugins.style + '")' + //
        '},' + //
        '"@knot/jsx":require("' + options.plugins.jsx + '"),' + //
        '"@knot/style":require("' + options.plugins.style + '")' + //
        '},require("' + options.plugins.utils + '"))';

      callback(null, wrapped);
    })
    .catch(callback);
};