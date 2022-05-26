import { fromEntries } from '@knot/common';
import Emittery from 'emittery';
import execa from 'execa';
import {
  isJSONRPCRequest,
  isJSONRPCResponse,
  JSONRPCClient,
} from 'json-rpc-2.0';

import { NotificationMap } from '../protocol';

const NEWLINE = '\r\n';
const CONTENT_LENGTH_HEADER = 'Content-Length';
const HEADER_SEPARATOR = ': ';
const LINE_BREAK = NEWLINE + NEWLINE;

export interface RPCOptions {
  /**
   * print all incoming and outgoing RPC messages
   */
  debug?: boolean;
}

class RPCClient extends Emittery<NotificationMap> {
  private rpc = new JSONRPCClient(async (rpcRequest) => {
    const json = JSON.stringify(rpcRequest);

    if (this.options.debug) {
      console.log(
        `sending '${rpcRequest.method ?? 'anonymous'}' request`,
        json
      );
    }

    const message = `${CONTENT_LENGTH_HEADER}${HEADER_SEPARATOR}${json.length}${LINE_BREAK}${json}`;

    this.proc.stdin.write(message, (err) => {
      if (err) {
        console.error(err);
      }
    });
  });

  request = this.rpc.request.bind(this.rpc);
  notify = this.rpc.notify.bind(this.rpc);

  constructor(
    private proc: execa.ExecaChildProcess,
    private options: RPCOptions = {}
  ) {
    super();

    proc.stdin.setDefaultEncoding('utf8');

    proc.stdout.on('data', (data) => {
      const dataStr = Buffer.from(data).toString();

      this.read(dataStr);
    });
  }

  private read(data: string) {
    const headerBreak = data.indexOf(LINE_BREAK);
    const rawHeaders = data.slice(0, headerBreak);
    const rest = data.slice(headerBreak + LINE_BREAK.length);

    const headers = fromEntries(
      rawHeaders
        .split(NEWLINE)
        .map(
          (rawHeader) => rawHeader.split(HEADER_SEPARATOR) as [string, string]
        )
    );

    const contentLength = Number(headers[CONTENT_LENGTH_HEADER] ?? 0);
    const content = rest.slice(0, contentLength);
    const message = JSON.parse(content);

    if (isJSONRPCRequest(message)) {
      this.emit<any>(message.method, message.params);

      if (this.options.debug) {
        console.log(`received '${message.method}' request`, content);
      }
    } else if (isJSONRPCResponse(message)) {
      this.rpc.receive(message);

      if (this.options.debug) {
        console.log(`received response #${message.id}`, content);
      }
    }

    if (rest?.length > contentLength) {
      this.read(rest.slice(contentLength));
    }
  }

  terminate() {
    this.rpc.rejectAllPendingRequests('terminated');
    this.proc.kill('SIGTERM', { forceKillAfterTimeout: 1500 });
  }
}

export default RPCClient;
