import { JSONRPCSuccessResponse } from 'json-rpc-2.0';

export enum Method {
  INITIALIZE = 'initialize'
}

export interface InitializeParams {
  root_dir: string;
}

export interface InitializeResult {}
