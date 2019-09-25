type execution_error =
  | MissingRootDirectory
  | InvalidPathFormat(string)
  | InvalidEntryPoint(string)
  | ModuleDoesNotExist(string, string)
  | EntryPointOutsideBuildContext(string, string);

exception ExecutionError(execution_error);
exception InternalCompilationError;

let throw_exec = e => raise(ExecutionError(e));
