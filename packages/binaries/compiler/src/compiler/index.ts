/* eslint-disable @typescript-eslint/camelcase */

import { OptionOverrides, Options } from '../types';
import Client from './client';
import { DEFAULT_OPTIONS, INFINITE_ATTEMPTS } from './constants';
import { ModuleStatus, Status } from './protocol';
import { pollingPromise } from './utils';

class Compiler {
  public options: Options;
  public isReady = false;
  public isRunning = false;
  private client: Client;

  constructor(options: OptionOverrides) {
    this.options = {
      ...DEFAULT_OPTIONS,
      ...options,
      plugins: {
        ...DEFAULT_OPTIONS.plugins,
        ...options.plugins
      }
    };

    this.client = new Client({
      knotc: this.options.knotc,
      cwd: this.options.cwd,
      target: this.options.target,
      config: this.options.config,
      debug: this.options.debug
    });

    this.isRunning = true;
  }

  private async awaitStatus(targetStatus: Status): Promise<void> {
    return pollingPromise((resolve, reject) =>
      this.client
        .status()
        .then(({ status }) => (status === targetStatus ? resolve() : reject()))
        .catch(reject)
    );
  }

  public async awaitComplete(): Promise<void> {
    return this.awaitStatus(Status.RUNNING);
  }

  public async awaitIdle(): Promise<void> {
    return this.awaitStatus(Status.IDLE);
  }

  public async awaitReady(): Promise<void> {
    if (!this.isReady) {
      await this.awaitIdle();

      this.isReady = true;
    }
  }

  public close(): void {
    if (!this.isRunning) return;

    this.isRunning = false;
    this.client.terminate();
  }

  /* module methods */

  public async add(path: string): Promise<void> {
    await this.awaitReady();

    return this.client.addModule({ path });
  }

  public async awaitModule(path: string): Promise<void> {
    return pollingPromise(
      (resolve, reject, abort) =>
        this.client.moduleStatus({ path }).then(({ status }) => {
          switch (status) {
            case ModuleStatus.VALID:
            case ModuleStatus.PARTIAL:
              return resolve();
            case ModuleStatus.INVALID:
              return abort(new Error(`module "${path}" failed to compile`));
            default:
              return reject();
          }
        }),
      INFINITE_ATTEMPTS,
      50
    );
  }

  public async fetch(path: string): Promise<string> {
    const { data } = await this.client.fetchModule({ path });

    return data;
  }

  public async update(path: string): Promise<void> {
    return this.client.updateModule({ path });
  }
}

export default Compiler;
