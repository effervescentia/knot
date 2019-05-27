// tslint:disable:no-expression-statement
import * as KnotCompiler from '@knot/compiler';
import execa from 'execa';
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
  const proc = execa(options.knot || KnotCompiler.path, [...knotArgs]);
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
