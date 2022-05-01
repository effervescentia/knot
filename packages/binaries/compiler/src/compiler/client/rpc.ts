import execa from 'execa';
import { JSONRPCClient } from 'json-rpc-2.0';

const LINE_BREAK = '\r\n\r\n';

export interface RPCOptions {
  proc: execa.ExecaChildProcess;
  /**
   * path to the binary being executed
   */
  cmd: string;
  /**
   * arguments to pass to `cmd`
   */
  args?: string[];
}

const createRPC = (proc: execa.ExecaChildProcess) => {
  proc.stderr.on('data', data => console.error(data.toString()));

  const rpc = new JSONRPCClient(async rpcRequest => {
    const json = JSON.stringify(rpcRequest);
    console.error(json);

    proc.stdin.write(
      `Content-Length: ${json.length}${LINE_BREAK}${json}`,
      'utf8'
    );
  });

  proc.stdout.on('data', data => {
    const dataStr = data.toString();
    const [, content] = dataStr.split(LINE_BREAK);

    rpc.receive(JSON.parse(content));
  });

  return Object.assign(rpc, {
    terminate() {
      proc.kill('SIGTERM', { forceKillAfterTimeout: 1500 });
    }
  });
};

export default createRPC;
