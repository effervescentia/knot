/**
 Errors and utitilies for problems encountered while compiling.
 */
open Infix;

exception NotImplemented;

exception ParseFailed;
exception WatchFailed(string);

type parse_err =
  | TypeError(Type.type_err);

type compile_err =
  | ImportCycle(list(string))
  | UnresolvedModule(string)
  | FileNotFound(string)
  | ParseError(parse_err);

exception CompilerError(list(compile_err));

let throw = err => raise(CompilerError([err]));

let throw_all = errs => raise(CompilerError(errs));

let _parse_err_to_string =
  fun
  | TypeError(err) =>
    err |> Type.err_to_string |> Print.fmt("type error: %s");

let _compile_err_to_string =
  fun
  | ImportCycle(cycles) =>
    cycles
    |> Print.many(~separator=" -> ", Functional.identity)
    |> Print.fmt("import cycle between the following modules: %s")
  | UnresolvedModule(name) =>
    name |> Print.fmt("could not resolve module: %s")
  | FileNotFound(path) =>
    path |> Print.fmt("could not find file with path: %s")
  | ParseError(err) =>
    err |> _parse_err_to_string |> Print.fmt("error found while parsing: %s");

let print_errs =
  Print.many(~separator="\n\n", _compile_err_to_string)
  % Print.fmt("found some errors during compilation:\n\n%s");
