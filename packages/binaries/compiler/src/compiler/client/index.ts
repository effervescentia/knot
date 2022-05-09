import execa from 'execa';
import { JSONRPCClient } from 'json-rpc-2.0';

import { KNOTC_BINARY } from '../../config';
import { Target } from '../../types';
import {
  Method,
  ModuleAddParams,
  ModuleFetchParams,
  ModuleFetchResult,
  ModuleInvalidateParams,
  ModuleStatusParams,
  ModuleStatusResult,
  StatusResult
} from '../protocol';
import createRPC from './rpc';

export interface ClientOptions {
  /**
   * path to the `knotc` binary
   */
  knotc: string;
  /**
   * root directory of the target knot project
   */
  rootDir: string;
  /**
   * the compilation target
   */
  target: Target;
  /**
   * location of the knot config file [default `.knot.yml`]
   */
  config: string;
}

class Client {
  private static handleError(message: string): (err: Error) => never {
    return err => {
      console.error(message, err);

      throw new Error(message);
    };
  }

  private proc: execa.ExecaChildProcess;
  private rpc: JSONRPCClient;

  constructor(options: ClientOptions) {
    const knotArgs = [
      'build_serve',
      '--root-dir',
      options.rootDir,
      '--config',
      options.config
    ];
    const [cmd, ...args] = (options.knotc || KNOTC_BINARY).split(/\s+/);
    const allArgs = [...args, ...knotArgs];

    console.log('starting compiler');
    console.log(`> ${[cmd, ...allArgs].join(' ')}`);

    this.proc = execa(cmd, allArgs);
    this.rpc = createRPC(this.proc);
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

  public async invalidateModule(params: ModuleInvalidateParams): Promise<void> {
    return this.rpc.notify(Method.MODULE_INVALIDATE, params);
  }

  public async reset(): Promise<void> {
    return this.rpc.request(Method.RESET);
  }

  public async status(): Promise<StatusResult> {
    return this.rpc.request(Method.STATUS);
  }

  public terminate(): void {
    this.rpc.rejectAllPendingRequests('terminated');
    this.proc.kill('SIGTERM', { forceKillAfterTimeout: 2000 });
  }
}

export default Client;
