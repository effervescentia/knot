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

  const opts = {
    cwd: __dirname + '/../compiler'
  };
  const result = options.knot ? execa.sync(options.knot, [file.name], opts) : execa.sync('esy', ['x', 'Compiler.exe', file.name], opts);

  console.log(result.stdout);
  console.error(result.stderr);

  return 'module.exports=(' + result.stdout + ')({' + //
    '["@knot/jsx"]:require("@knot/react-plugin"),' + //
    '["@knot/style"]:require("@knot/css-plugin")' + //
    '},require("@knot/javascript-utils"))';
}