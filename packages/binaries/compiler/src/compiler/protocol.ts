export enum Method {
  MODULE_FETCH = 'module/fetch',
  MODULE_STATUS = 'module/status',
  MODULE_ADD = 'module/add',
  MODULE_UPDATE = 'module/update',
  MODULE_REMOVE = 'module/remove',

  STATUS = 'compiler/status',
  RESET = 'compiler/reset',
  ERROR = 'compiler/error'
}

export interface NotificationMap {
  [Method.ERROR]: CompilerErrorParams;
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
  NONE = 'none',
  PENDING = 'pending',
  PURGED = 'purged',
  VALID = 'valid',
  PARTIAL = 'partial',
  INVALID = 'invalid'
}

export type ModuleStatusParams = ModuleParams;

export interface ModuleStatusResult {
  status: ModuleStatus;
}

/* compiler/status */

export enum Status {
  IDLE = 'idle',
  RUNNING = 'running'
}

export interface StatusResult {
  status: Status;
}

/* module/add */

export type ModuleAddParams = ModuleParams;

/* module/update */

export type ModuleUpdateParams = ModuleParams;

/* module/remove */

export type ModuleRemoveParams = ModuleParams;

/* compiler/error */

export enum CompilerErrorType {
  FILE_NOT_FOUND = 'file_not_found',
  UNRESOLVED_MODULE = 'unresolved_module',
  INVALID_MODULE = 'invalid_module',
  IMPORT_CYCLE = 'import_cycle'
}

export enum ModuleErrorType {}

export interface BaseError {
  message: string;
}

export interface CompilerError extends BaseError {
  type: CompilerErrorType;
}

export interface ModuleError extends BaseError {
  type: ModuleErrorType;
  path: string;
}

export type AnyError = CompilerError | ModuleError;

export interface CompilerErrorParams {
  errors: AnyError[];
}
