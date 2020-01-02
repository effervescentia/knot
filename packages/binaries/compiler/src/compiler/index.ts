// tslint:disable:no-expression-statement
import execa from 'execa';
import { KNOT_BINARY } from '../config';
import { Options } from '../types';
import { DEFAULT_OPTIONS } from './constants';
import * as Tasks from './tasks';

interface FullOptions extends Options {
  readonly baseUrl: string;
}

function startCompiler(options: FullOptions): void {
  const knotArgs: ReadonlyArray<any> = [
    '-server',
    '-port',
    options.port,
    '-compiler.module',
    options.compiler.module,
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

  // tslint:disable-next-line:no-console
  proc.stdout.on('data', data => console.log(data.toString()));
  // tslint:disable-next-line:no-console
  proc.stderr.on('data', data => console.error(data.toString()));
}

class Compiler {
  // tslint:disable: readonly-keyword
  public options: FullOptions;
  public isReady = false;
  public isRunning = false;
  // tslint:enable: readonly-keyword

  constructor(options: Partial<Options>) {
    const mergedOptions: Options = {
      ...DEFAULT_OPTIONS,
      ...options,
      compiler: {
        ...DEFAULT_OPTIONS.compiler,
        ...options.compiler
      },
      plugins: {
        ...DEFAULT_OPTIONS.plugins,
        ...options.plugins
      }
    };

    this.options = {
      ...mergedOptions,
      baseUrl: `http://localhost:${mergedOptions.port}`
    };

    startCompiler(this.options);

    this.isRunning = true;
  }

  public async add(path: string): Promise<void | Response> {
    await this.awaitReady();

    return Tasks.addModule(this.options, path);
  }

  public awaitComplete(): Promise<void> {
    return Tasks.awaitCompilationComplete(this.options);
  }

  public awaitModule(path: string): Promise<void> {
    return Tasks.awaitModuleComplete(this.options, path);
  }

  public close(): Promise<void | Response> {
    if (!this.isRunning) {
      return Promise.resolve();
    }

    // tslint:disable-next-line: no-object-mutation
    this.isRunning = false;

    return Tasks.killServer(this.options);
  }

  public generate(path: string): Promise<void | string> {
    return Tasks.generateModule(this.options, path);
  }

  public invalidate(path: string): Promise<void | Response> {
    return Tasks.invalidateModule(this.options, path);
  }

  public awaitIdle(): Promise<void> {
    return Tasks.awaitServerIdle(this.options);
  }

  public async awaitReady(): Promise<void> {
    if (!this.isReady) {
      await this.awaitIdle();

      // tslint:disable-next-line: no-object-mutation
      this.isReady = true;
    }
  }
}

export default Compiler;
