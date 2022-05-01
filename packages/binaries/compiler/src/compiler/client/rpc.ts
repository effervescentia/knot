import execa from 'execa';
import { JSONRPCClient } from 'json-rpc-2.0';

export interface RPCOptions {
  /**
   * path to the binary being executed
   */
  cmd: string;
  /**
   * arguments to pass to `cmd`
   */
  args?: string[];
}

const createRPC = ({ cmd, args = [] }: RPCOptions) => {
  const proc = execa(cmd, args);

  proc.stderr.on('data', data => console.error(data.toString()));

  const rpc = new JSONRPCClient(async rpcRequest => {
    const json = JSON.stringify(rpcRequest);
    console.error(json);

    proc.stdin.write(`Content-Length: ${json.length}\r\n\r\n${json}`, 'utf8');
  });

  proc.stdout.on('data', data => {
    const dataStr = data.toString();
    const [, content] = dataStr.split('\r\n\r\n');

    rpc.receive(JSON.parse(content));
  });

  return rpc;
};

export default createRPC;
