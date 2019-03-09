const {
  getOptions
} = require('loader-utils');
const execa = require('execa');
const tmp = require('tmp');
const fs = require('fs');

const GLOBALS = '$$knot_globals$$'
const DEFAULT_OPTIONS = {
  plugins: {
    jsx: '@knot/react-plugin',
    style: '@knot/css-plugin',
    utils: '@knot/javascript-utils'
  }
};

module.exports = function loader(source) {
  const options = Object.assign({}, DEFAULT_OPTIONS, getOptions(this));

  const file = tmp.fileSync();
  fs.writeFileSync(file.name, source);

  const result = options.knot ? execa.sync(options.knot, [file.name]) : execa.sync('esy', ['x', 'knotc.exe', file.name], {
    cwd: __dirname + '/../compiler'
  });

  console.log(result.stdout);
  console.error(result.stderr);

  return 'module.exports=(' + result.stdout + ')({' + //
    '"' + GLOBALS + '":{' + //
    'jsx:require("' + options.plugins.jsx + '"),' + //
    'style:require("' + options.plugins.style + '")' + //
    '},' + //
    '"@knot/jsx":require("' + options.plugins.jsx + '"),' + //
    '"@knot/style":require("' + options.plugins.style + '")' + //
    '},require("' + options.plugins.utils + '"))';
}