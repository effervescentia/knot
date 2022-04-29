import { JSONRPCClient } from 'json-rpc-2.0';
import process from 'process';

import { OptionOverrides, Options } from '../types';
import createClient from './client';
import { DEFAULT_OPTIONS } from './constants';
import * as Tasks from './tasks';

interface FullOptions extends Options {
  readonly baseUrl: string;
}

class Compiler {
  public options: FullOptions;
  public isReady = false;
  public isRunning = false;
  private client: JSONRPCClient;

  constructor(options: OptionOverrides) {
    const mergedOptions: Options = {
      ...DEFAULT_OPTIONS,
      ...options,
      plugins: {
        ...DEFAULT_OPTIONS.plugins,
        ...options.plugins
      }
    };

    this.options = {
      ...mergedOptions,
      baseUrl: `http://localhost:${mergedOptions.port}`
    };

    this.client = createClient({
      knotc: '',
      rootDir: this.options.rootDir,
      config: this.options.config
    });

    this.client.send<{}>('initialize', {
      processId: process.pid
    });

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

      this.isReady = true;
    }
  }
}

export default Compiler;
