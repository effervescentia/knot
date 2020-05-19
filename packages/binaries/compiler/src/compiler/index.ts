import execa from 'execa';

import { KNOT_BINARY } from '../config';
import { OptionOverrides, Options } from '../types';
import wrapModule from '../wrapper';
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
  console.log(
    `running knot using the command "${[cmd, ...args, ...knotArgs].join(' ')}"`
  );

  const proc = execa(cmd, [...args, ...knotArgs]);

  proc.stdout.on('data', data => console.log(data.toString()));
  proc.stderr.on('data', data => console.error(data.toString()));
}

class Compiler {
  public options: FullOptions;
  public isReady = false;
  public isRunning = false;

  constructor(options: OptionOverrides) {
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

    this.isRunning = false;

    return Tasks.killServer(this.options);
  }

  public async generate(path: string): Promise<void | string> {
    const generated = await Tasks.generateModule(this.options, path);

    if (generated) {
      return wrapModule(generated, this.options);
    }

    return generated;
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

      this.isReady = true;
    }
  }
}

export default Compiler;
