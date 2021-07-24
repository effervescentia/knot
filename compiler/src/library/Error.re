/**
 Errors and utitilies for problems encountered while compiling.
 */
open Infix;
open Reference;

exception NotImplemented;

exception WatchFailed(string);

type parse_err =
  | TypeError(Type2.Raw.error_t)
  | ReservedKeyword(string);

type compile_err =
  | ImportCycle(list(string))
  | UnresolvedModule(string)
  | FileNotFound(string)
  | ParseError(parse_err, Namespace.t, Cursor.t)
  | InvalidModule(Namespace.t);

exception CompileError(list(compile_err));

let throw = err => raise(CompileError([err]));

let throw_all = errs => raise(CompileError(errs));

let parse_err_to_string =
  fun
  | TypeError(err) =>
    err
    |> Type2.Error.to_string(Type2.Raw.to_string)
    |> Print.fmt("type error: %s")
  | ReservedKeyword(name) =>
    name |> Print.fmt("reserved keyword %s cannot be used as an identifier");

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
  | ParseError(err, namespace, cursor) =>
    Print.fmt(
      "error found while parsing %s: %s",
      namespace |> Namespace.to_string,
      err |> parse_err_to_string,
    )
  | InvalidModule(namespace) =>
    namespace
    |> Namespace.to_string
    |> Print.fmt("failed to parse module: %s");

let print_errs =
  Print.many(~separator="\n\n", _compile_err_to_string)
  % Print.fmt("found some errors during compilation:\n\n%s");
