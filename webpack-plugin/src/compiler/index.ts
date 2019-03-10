import execa from 'execa';
import * as knotBinary from '@knot/binary';
import * as Tasks from './tasks';

export default function createCompiler(options) {
  const execaOpts = {
    cwd: knotBinary.path
  };
  const knotArgs = [
    '-server',
    '-port',
    options.port,
    '-config',
    options.config,
    ...(options.debug ? ['-debug'] : [])
  ];
  const proc = options.knot
    ? execa(options, [...knotArgs], execaOpts)
    : execa('esy', ['x', 'knotc.exe', ...knotArgs], execaOpts);
  const baseUrl = `http://localhost:${options.port}`;

  proc.stdout.on('data', data => console.log(data.toString()));
  proc.stderr.on('data', data => console.error(data.toString()));

  const fullOptions = {
    ...options,
    baseUrl
  };

  return {
    await: Tasks.awaitServerIdle(fullOptions),
    awaitComplete: Tasks.awaitComilationComplete(fullOptions),
    generate: Tasks.generateModule(fullOptions),
    add: Tasks.addModule(fullOptions),
    reset: Tasks.resetContext(fullOptions),
    close: Tasks.killServer(fullOptions)
  };
}
