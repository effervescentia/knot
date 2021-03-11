/**
 Errors and utitilies for problems encountered while compiling.
 */
open Infix;

exception NotImplemented;

exception ParseFailed;
exception WatchFailed(string);

type compiler_err =
  | ImportCycle(list(string))
  | UnresolvedModule(string)
  | FileNotFound(string);

exception CompilerError(list(compiler_err));

let throw = err => raise(CompilerError([err]));

let throw_all = errs => raise(CompilerError(errs));

let _print_err =
  fun
  | ImportCycle(cycles) =>
    cycles
    |> Print.many(~separator=" -> ", Functional.identity)
    |> Print.fmt("import cycle between the following modules: %s")
  | UnresolvedModule(name) =>
    name |> Print.fmt("could not resolve module: %s")
  | FileNotFound(path) =>
    path |> Print.fmt("could not find file with path: %s");

let print_errs =
  Print.many(~separator="\n\n", _print_err)
  % Print.fmt("found some errors during compilation:\n\n%s");
