declare module 'budo' {
  import * as stream from 'stream';
  import * as http from 'http';
  import { EventEmitter } from 'events';
  import * as Browserify from 'browserify';

  interface Options {
    port?: number;
    live?: boolean;
    open?: boolean;
    browserify?: Browserify.Options;
  }

  interface ConnectPayload {
    uri: string;
    serve: string;
    dir: string;
    host: string;
    port: number;
    entries: string[];
    server: http.Server;
  }

  interface BudoInstance extends EventEmitter {
    on(event: 'connect', callback: (payload: ConnectPayload) => void): this;
    on(event: 'update', callback: (buffer: Buffer) => void): this;
    on(event: 'error', callback: (payload: Error) => void): this;
    on(event: string, callback: () => void): this;
  }

  interface BudoStatic {
    (entry?: string, options?: Options): BudoInstance;
    (options: Options): BudoInstance;
  }

  const budo: (options: Options) => BudoInstance;

  export = budo;
}
