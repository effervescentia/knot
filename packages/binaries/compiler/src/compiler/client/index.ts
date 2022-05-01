import { JSONRPCClient } from 'json-rpc-2.0';

import { KNOTC_BINARY } from '../../config';
import { InitializeParams, Method } from '../protocol';
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
   * location of the knot config file [default `.knot.yml`]
   */
  config: string;
}

class Client {
  private rpc: JSONRPCClient;

  constructor(options: ClientOptions) {
    const knotArgs = [
      'lang_serve',
      '--root-dir',
      options.rootDir,
      '--config',
      options.config
    ];
    const [cmd, ...args] = (options.knotc || KNOTC_BINARY).split(/\s+/);
    const allArgs = [...args, ...knotArgs];

    console.log('starting language server');
    console.log(`> ${[cmd, ...allArgs].join(' ')}`);

    this.rpc = createRPC({ cmd, args: allArgs });
  }

  async initialize(params: InitializeParams): Promise<{}> {
    return this.rpc.request(Method.INITIALIZE, params);
  }
}

export default Client;
