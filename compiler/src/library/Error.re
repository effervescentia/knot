/**
 * Errors and utitilies for problems encountered while compiling.
 */
open Infix;

exception NotImplemented;

exception ParseFailed;
exception WatchFailed(string);

type compiler_err =
  | ErrorList(list(compiler_err))
  | ImportCycle(list(string))
  | UnresolvedModule(string);

exception CompilerError(compiler_err);

let throw = err => raise(CompilerError(err));

let throw_all = errs => throw(ErrorList(errs));

let rec _print_err =
  fun
  | ImportCycle(cycles) =>
    cycles
    |> Print.many(~separator=" -> ", Functional.identity)
    |> Print.fmt("import cycle between the following modules: %s")
  | UnresolvedModule(path) =>
    path |> Print.fmt("could not resolve module with path: %s")
  | ErrorList(errors) => errors |> Print.many(~separator="\n\n", _print_err);

let print_err =
  _print_err % Print.fmt("found some errors during compilation:\n\n%s");
