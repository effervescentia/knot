// tslint:disable:no-expression-statement
import execa from 'execa';
import { KNOT_BINARY } from '../config';
import { Compiler, Options } from '../types';
import * as Tasks from './tasks';

export default function createCompiler(options: Options): Compiler {
  const knotArgs: ReadonlyArray<any> = [
    '-server',
    '-port',
    options.port,
    '-config',
    options.config,
    ...(options.debug ? ['-debug'] : [])
  ];
  const [cmd, ...args] = (options.knot || KNOT_BINARY).split(/\s+/);
  // tslint:disable-next-line: no-console
  console.log(
    `running knot using the command "${[cmd, ...args, ...knotArgs].join(' ')}"`
  );

  const proc = execa(cmd, [...args, ...knotArgs]);
  const baseUrl = `http://localhost:${options.port}`;

  // tslint:disable-next-line:no-console
  proc.stdout.on('data', data => console.log(data.toString()));
  // tslint:disable-next-line:no-console
  proc.stderr.on('data', data => console.error(data.toString()));

  const fullOptions = {
    ...options,
    baseUrl
  };

  return {
    add: Tasks.addModule(fullOptions),
    await: Tasks.awaitServerIdle(fullOptions),
    awaitComplete: Tasks.awaitComilationComplete(fullOptions),
    awaitModule: Tasks.awaitModuleComplete(fullOptions),
    close: Tasks.killServer(fullOptions),
    generate: Tasks.generateModule(fullOptions),
    invalidate: Tasks.invalidateModule(fullOptions)
  };
}
