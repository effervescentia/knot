import { noop } from '@knot/common';
import chalk from 'chalk';
import execa from 'execa';

import { KNOTC_BINARY } from '../../config';
import { Target } from '../../types';
import {
  Method,
  ModuleAddParams,
  ModuleFetchParams,
  ModuleFetchResult,
  ModuleRemoveParams,
  ModuleStatusParams,
  ModuleStatusResult,
  ModuleUpdateParams,
  StatusResult
} from '../protocol';
import createRPC, { RPCClient } from './rpc';

export interface ClientOptions {
  /**
   * working directory for running the `knotc` binary
   */
  cwd: string;
  /**
   * path to the `knotc` binary
   */
  knotc?: string;
  /**
   * the compilation target
   */
  target?: Target;
  /**
   * location of the knot config file [default `.knot.yml`]
   */
  config?: string;
  /**
   * enable debug logs from `knotc`
   */
  debug?: boolean;
}

class Client {
  private static handleError(message: string): (err: Error) => never {
    return err => {
      console.error(message, err);

      throw new Error(message);
    };
  }

  private proc: execa.ExecaChildProcess;
  private rpc: RPCClient;
  private unsubscribeErrorHandler: VoidFunction = noop;

  constructor(options: ClientOptions) {
    const knotArgs = [
      'build_serve',
      '--cwd',
      options.cwd,
      ...(options.config ? ['--config', options.config] : []),
      ...(options.target ? ['--target', options.target] : []),
      ...(options.debug ? ['--debug', '--log-imports'] : [])
    ];
    const [cmd, ...args] = (options.knotc || KNOTC_BINARY).split(/\s+/);
    const allArgs = [...args, ...knotArgs];

    if (options.debug) {
      console.error('starting compiler');
      console.error(`> ${[cmd, ...allArgs].join(' ')}`);
    }

    this.proc = execa(cmd, allArgs);
    this.rpc = createRPC(this.proc);

    if (!options.debug) return;

    this.unsubscribeErrorHandler = this.rpc.emitter.on(
      Method.ERROR,
      ({ errors }) => {
        errors.forEach(error =>
          console.error(`ERROR ${error.type}: ${chalk.red(error.message)}`)
        );
      }
    );

    this.proc.stderr.on('data', data => console.error(data.toString()));
  }

  public async fetchModule(
    params: ModuleFetchParams
  ): Promise<ModuleFetchResult> {
    return this.rpc
      .request(Method.MODULE_FETCH, params)
      .then(
        undefined,
        Client.handleError('unable to fetch module from compiler')
      );
  }

  public async moduleStatus(
    params: ModuleStatusParams
  ): Promise<ModuleStatusResult> {
    return this.rpc.request(Method.MODULE_STATUS, params);
  }

  public async addModule(params: ModuleAddParams): Promise<void> {
    return this.rpc.notify(Method.MODULE_ADD, params);
  }

  public async updateModule(params: ModuleUpdateParams): Promise<void> {
    return this.rpc.notify(Method.MODULE_UPDATE, params);
  }

  public async removeModule(params: ModuleRemoveParams): Promise<void> {
    return this.rpc.notify(Method.MODULE_REMOVE, params);
  }

  public async reset(): Promise<void> {
    return this.rpc.request(Method.RESET);
  }

  public async status(): Promise<StatusResult> {
    return this.rpc.request(Method.STATUS);
  }

  public terminate(): void {
    this.unsubscribeErrorHandler();
    this.unsubscribeErrorHandler = noop;
    this.rpc.rejectAllPendingRequests('terminated');
    this.proc.kill('SIGTERM', { forceKillAfterTimeout: 2000 });
  }
}

export default Client;
