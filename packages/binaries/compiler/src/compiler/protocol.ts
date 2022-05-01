export enum Method {
  MODULE_FETCH = 'module/fetch',
  MODULE_STATUS = 'module/status',
  MODULE_ADD = 'module/add',
  MODULE_INVALIDATE = 'module/invalidate',

  STATUS = 'compiler/status',
  RESET = 'compiler/reset'
}

export interface ModuleParams {
  path: string;
}

/* module/fetch */

export type ModuleFetchParams = ModuleParams;

export interface ModuleFetchResult {
  data: string;
}

/* module/status */

export enum ModuleStatus {
  PENDING = 'pending',
  COMPLETE = 'complete',
  FAILED = 'failed'
}

export type ModuleStatusParams = ModuleParams;

export interface ModuleStatusResult {
  status: ModuleStatus;
}

/* compiler/status */

export enum Status {
  IDLE = 'idle',
  COMPLETE = 'complete'
}

export interface StatusResult {
  status: Status;
}

/* module/add */

export type ModuleAddParams = ModuleParams;

/* module/invalidate */

export type ModuleInvalidateParams = ModuleParams;
