import { noop } from '@knot/common';
import chalk from 'chalk';
import execa from 'execa';

import { KNOTC_BINARY } from '../../config';
import { Target } from '../../types';
import {
  AnyError,
  Method,
  ModuleAddParams,
  ModuleFetchParams,
  ModuleFetchResult,
  ModuleRemoveParams,
  ModuleStatusParams,
  ModuleStatusResult,
  ModuleUpdateParams,
  StatusResult,
} from '../protocol';
import RPCClient from './rpc';

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
    return (err) => {
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
      ...(options.debug ? ['--debug', '--log-imports'] : []),
    ];
    const [cmd, ...args] = (options.knotc || KNOTC_BINARY).split(/\s+/);
    const allArgs = [...args, ...knotArgs];

    if (options.debug) {
      console.error('starting compiler');
      console.error(`> ${[cmd, ...allArgs].join(' ')}`);
    }

    this.proc = execa(cmd, allArgs);
    this.rpc = new RPCClient(this.proc, { debug: options.debug });

    if (!options.debug) return;

    this.replaceErrorHandler((error) =>
      console.error(`ERROR ${error.type}: ${chalk.red(error.message)}`)
    );

    this.proc.stderr.on('data', (data) => console.error(data.toString()));
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

  public removeErrorHandler() {
    this.unsubscribeErrorHandler();
    this.unsubscribeErrorHandler = noop;
  }

  public replaceErrorHandler(listener: (error: AnyError) => void) {
    this.removeErrorHandler();

    this.unsubscribeErrorHandler = this.unsubscribeErrorHandler = this.rpc.on(
      Method.ERROR,
      ({ errors }) => errors.forEach(listener)
    );
  }

  public terminate(): void {
    this.removeErrorHandler();
    this.rpc.terminate();
  }
}

export default Client;
