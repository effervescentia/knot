import { fromEntries } from '@knot/common';
import Emittery from 'emittery';
import execa from 'execa';
import {
  isJSONRPCRequest,
  isJSONRPCResponse,
  JSONRPCClient
} from 'json-rpc-2.0';

import { NotificationMap } from '../protocol';

const NEWLINE = '\r\n';
const CONTENT_LENGTH_HEADER = 'Content-Length';
const HEADER_SEPARATOR = ': ';
const LINE_BREAK = NEWLINE + NEWLINE;

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

export interface RPCClient extends JSONRPCClient {
  emitter: Emittery<NotificationMap>;
  terminate(): void;
}

const createRPC = (proc: execa.ExecaChildProcess): RPCClient => {
  const emitter = new Emittery<NotificationMap>();
  const rpc = new JSONRPCClient(async rpcRequest => {
    const json = JSON.stringify(rpcRequest);
    const message = `${CONTENT_LENGTH_HEADER}${HEADER_SEPARATOR}${json.length}${LINE_BREAK}${json}`;

    proc.stdin.write(message, 'utf-8');
  });

  const read = (data: string) => {
    const headerBreak = data.indexOf(LINE_BREAK);
    const rawHeaders = data.slice(0, headerBreak);
    const rest = data.slice(headerBreak + LINE_BREAK.length);

    const headers = fromEntries(
      rawHeaders
        .split(NEWLINE)
        .map(rawHeader => rawHeader.split(HEADER_SEPARATOR) as [string, string])
    );

    const contentLength = Number(headers[CONTENT_LENGTH_HEADER] ?? 0);
    const content = rest.slice(0, contentLength);
    const message = JSON.parse(content);

    if (isJSONRPCRequest(message)) {
      emitter.emit<any>(message.method, message.params);
    } else if (isJSONRPCResponse(message)) {
      rpc.receive(message);
    }

    if (rest?.length > contentLength) {
      read(rest.slice(contentLength));
    }
  };

  proc.stdout.on('data', data => {
    const dataStr = Buffer.from(data).toString();

    read(dataStr);
  });

  return Object.assign(rpc, {
    emitter,
    terminate() {
      proc.kill('SIGTERM', { forceKillAfterTimeout: 1500 });
    }
  });
};

export default createRPC;
