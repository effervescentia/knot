import { fromEntries } from '@knot/common';
import Emittery from 'emittery';
import execa from 'execa';
import {
  isJSONRPCRequest,
  isJSONRPCResponse,
  JSONRPCClient,
  JSONRPCParams,
} from 'json-rpc-2.0';

import { CONNECTION_TIMEOUT } from '../constants';
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

class RPCClient
  extends Emittery<NotificationMap>
  implements Pick<JSONRPCClient, 'request' | 'notify'>
{
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

  private connecting: Promise<void> | null;

  constructor(
    private readonly proc: execa.ExecaChildProcess,
    private readonly options: RPCOptions = {}
  ) {
    super();
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

  request(
    method: string,
    params?: JSONRPCParams,
    clientParams?: void
  ): PromiseLike<any> {
    if (this.options.debug) {
      console.log(`sending '${method}' request`, params);
    }

    return this.rpc.request(method, params, clientParams);
  }

  notify(method: string, params?: JSONRPCParams, clientParams?: void): void {
    if (this.options.debug) {
      console.log(`sending '${method}' notification`, params);
    }

    return this.rpc.notify(method, params, clientParams);
  }

  start() {
    if (this.connecting) {
      throw new Error('compiler client has already been started');
    }

    let isConnected = false;
    this.connecting = new Promise((resolve, reject) => {
      this.proc.once('spawn', () => {
        isConnected = true;
        resolve();

        this.proc.stdin.setDefaultEncoding('utf8');

        this.proc.stdout.on('data', (data) => {
          const dataStr = Buffer.from(data).toString();

          this.read(dataStr);
        });
      });

      setTimeout(() => {
        if (!isConnected) {
          reject(
            new Error('failed to establish connection with compiler process')
          );
        }
      }, CONNECTION_TIMEOUT);
    });

    return this.connecting;
  }

  terminate() {
    this.rpc.rejectAllPendingRequests('terminated');
    this.proc.kill('SIGTERM', { forceKillAfterTimeout: 1500 });
  }
}

export default RPCClient;
