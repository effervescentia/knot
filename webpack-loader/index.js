const {
  getOptions
} = require('loader-utils');
const execa = require('execa');
const tmp = require('tmp');
const fs = require('fs');

module.exports = function loader(source) {
  const options = getOptions(this);

  const file = tmp.fileSync();
  fs.writeFileSync(file.name, source);

  const result = options.knot ? execa.sync(options.knot, [file.name]) : execa.sync('esy', ['x', 'Compiler.exe', file.name], {
    cwd: __dirname + '/../compiler'
  });

  console.error(result.stderr);

  return 'var stylePlugin = require("@knot/css-plugin");' + //
    'module.exports=(' + result.stdout + ')({' + //
    '["@knot/jsx"]:require("@knot/react-plugin"),' + //
    '["@knot/style"]:stylePlugin' + //
    '},require("@knot/javascript-utils")(stylePlugin));';
}