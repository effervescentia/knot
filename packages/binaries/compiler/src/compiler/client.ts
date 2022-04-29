import execa from 'execa';
import { JSONRPCClient } from 'json-rpc-2.0';

import { KNOTC_BINARY } from '../config';

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

const createClient = (options: ClientOptions) => {
  const knotArgs: ReadonlyArray<any> = [
    'lsp',
    '--root-dir',
    options.rootDir,
    '--config',
    options.config
  ];
  const [cmd, ...args] = (options.knotc || KNOTC_BINARY).split(/\s+/);
  console.log(
    `running "${[cmd, ...args, ...knotArgs].join(
      ' '
    )}" to start language server`
  );

  const proc = execa(cmd, [...args, ...knotArgs]);

  proc.stderr.on('data', data => console.error(data.toString()));

  const rpcClient = new JSONRPCClient(async rpcRequest => {
    const json = JSON.stringify(rpcRequest);
    console.error(json);

    proc.stdin.write(`Content-Length: ${json.length}\r\n\r\n${json}`, 'utf8');
  });

  proc.stdout.on('data', data => {
    const dataStr = data.toString();
    const [, content] = dataStr.split('\r\n\r\n');

    rpcClient.receive(JSON.parse(content));
  });

  return rpcClient;
};

export default createClient;
