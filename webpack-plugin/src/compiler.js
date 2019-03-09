const execa = require('execa');
const path = require('path');
const fetch = require('./fetch');

const MAX_ATTEMPTS = 10;
const ATTEMPT_TIMEOUT = 1000;

module.exports = function createCompiler(options) {
  const execaOpts = {
    cwd: path.resolve(__dirname, '../../compiler')
  };
  const knotArgs = ['-server', '-port', options.port, '-config', options.config];
  const proc = options.knot ? execa(options, [...knotArgs], execaOpts) : execa('esy', ['x', 'knotc.exe', ...knotArgs], execaOpts);

  proc.stdout.on('data', data => console.log(data.toString()));
  proc.stderr.on('data', data => console.error(data.toString()));

  return {
    await: () => {
      const baseUrl = `http://localhost:${options.port}`;
      let attempts = 0;

      const awaitServerHealthy = (resolve, reject) => setTimeout(() => {
        attempts += 1;

        fetch(`${baseUrl}/status`)
          .then(() => resolve())
          .catch(() => {
            if (attempts === MAX_ATTEMPTS) {
              reject('knot compiler did not return a healthy response');
            } else {
              awaitServerHealthy(resolve, reject)
            }
          });
      }, ATTEMPT_TIMEOUT);

      return new Promise(awaitServerHealthy);
    },
    generate: async () => '',
    add: (path) => console.log(`add '${path}' to knot compiler`),
    close: () => proc.kill()
  };
}