/**
 Errors and utitilies for problems encountered while compiling.
 */
open Infix;
open Reference;

exception NotImplemented;

exception WatchFailed(string);

/**
 errors encountered while parsing
 */
type parse_err =
  | TypeError(Type.Raw.error_t)
  | ReservedKeyword(string);

/**
 errors encountered while compiling
 */
type compile_err =
  | ImportCycle(list(string))
  | UnresolvedModule(string)
  | FileNotFound(string)
  | ParseError(parse_err, Namespace.t, Range.t)
  | InvalidModule(Namespace.t);

exception CompileError(list(compile_err));

/**
 raise a single compiler error
 */
let throw = err => raise(CompileError([err]));

/**
 raise multple compiler errors
 */
let throw_all = errs => raise(CompileError(errs));

/* pretty printing */

let pp_parse_err: Fmt.t(parse_err) =
  ppf =>
    fun
    | TypeError(err) =>
      Fmt.pf(ppf, "type error: %a", Type.Error.pp(Type.Raw.pp), err)
    | ReservedKeyword(name) =>
      Fmt.pf(
        ppf,
        "reserved keyword %s cannot be used as an identifier",
        name,
      );

let pp_dump_parse_err: Fmt.t(parse_err) =
  ppf =>
    fun
    | TypeError(err) =>
      Fmt.pf(ppf, "TypeError<%a>", Type.(Error.pp(Raw.pp)), err)
    | ReservedKeyword(name) => Fmt.pf(ppf, "ReservedKeyword<%s>", name);

let pp_compile_err: Fmt.t(compile_err) =
  ppf =>
    fun
    | ImportCycle(cycles) =>
      Fmt.pf(
        ppf,
        "import cycle between the following modules: %a",
        Fmt.list(~sep=(ppf, ()) => Fmt.string(ppf, " -> "), Fmt.string),
        cycles,
      )
    | UnresolvedModule(name) =>
      Fmt.pf(ppf, "could not resolve module: %s", name)
    | FileNotFound(path) =>
      Fmt.pf(ppf, "could not find file with path: %s", path)
    | ParseError(err, namespace, _) =>
      Fmt.pf(
        ppf,
        "error found while parsing %a: %a",
        Namespace.pp,
        namespace,
        pp_parse_err,
        err,
      )
    | InvalidModule(namespace) =>
      Fmt.pf(ppf, "failed to parse module: %a", Namespace.pp, namespace);

let pp_dump_compile_err: Fmt.t(compile_err) =
  ppf =>
    fun
    | ImportCycle(cycles) =>
      Fmt.pf(
        ppf,
        "ImportCycle<%a>",
        Fmt.list(~sep=(ppf, ()) => Fmt.string(ppf, " -> "), Fmt.string),
        cycles,
      )
    | UnresolvedModule(name) => Fmt.pf(ppf, "UnresolvedModule<%s>", name)
    | FileNotFound(path) => Fmt.pf(ppf, "FileNotFound<%s>", path)
    | InvalidModule(namespace) =>
      Fmt.pf(ppf, "InvalidModule<%a>", Namespace.pp, namespace)
    | ParseError(err, namespace, range) =>
      Fmt.pf(
        ppf,
        "ParseError<%a, %a, %a>",
        pp_dump_parse_err,
        err,
        Namespace.pp,
        namespace,
        Range.pp,
        range,
      );

let pp_dump_err_list: Fmt.t(list(compile_err)) =
  Fmt.list(~sep=Fmt.comma, pp_dump_compile_err);

let pp_err_list: Fmt.t(list(compile_err)) =
  ppf =>
    Fmt.pf(
      ppf,
      "found some errors during compilation:\n\n%a",
      Fmt.list(~sep=(ppf, ()) => Fmt.string(ppf, "\n\n"), pp_compile_err),
    );
