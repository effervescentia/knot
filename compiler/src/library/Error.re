/**
 Errors and utilities for problems encountered while compiling.
 */
open Infix;
open Reference;

exception NotImplemented;

exception WatchFailed(string);

/**
 errors encountered while parsing
 */
type parse_err =
  | TypeError(Type.error_t)
  | ReservedKeyword(string);

/**
 errors encountered while compiling
 */
type compile_err =
  /* issues encountered while parsing and analyzing */
  | ParseError(parse_err, Namespace.t, Range.t)
  /* import cycle exists between modules */
  | ImportCycle(list(string))
  /* unable to retrieve a file from the filesystem */
  | FileNotFound(string)
  /* module could not be resolved */
  | UnresolvedModule(string)
  /* module did not contain any imports or declarations */
  | InvalidModule(Namespace.t);

exception CompileError(list(compile_err));

/**
 used to represent failures in the logic of the compiler itself
 */
exception SystemError;

let __arrow_sep = Fmt.Sep.(of_sep(~trail=Trail.nop, " ->"));

/**
 raise a single compiler error
 */
let throw = err => raise(CompileError([err]));

/**
 raise multiple compiler errors
 */
let throw_all = errs => raise(CompileError(errs));

/* pretty printing */

let pp_parse_err: Fmt.t(parse_err) =
  Fmt.(
    ppf =>
      fun
      | TypeError(err) => pf(ppf, "type error: %a", Type.pp_error, err)
      | ReservedKeyword(name) =>
        pf(ppf, "reserved keyword %s cannot be used as an identifier", name)
  );

let pp_dump_parse_err: Fmt.t(parse_err) =
  Fmt.(
    ppf =>
      fun
      | TypeError(err) => pf(ppf, "TypeError<%a>", Type.pp_error, err)
      | ReservedKeyword(name) => pf(ppf, "ReservedKeyword<%s>", name)
  );

let pp_compile_err: Fmt.t(compile_err) =
  ppf =>
    Fmt.(
      fun
      /* import cycles should always involve at least 1 module */
      | ImportCycle([]) => raise(SystemError)

      | ImportCycle([self_import]) =>
        pf(ppf, "the module %a imports itself", bold_str, self_import)

      | ImportCycle(cycles) =>
        pf(
          ppf,
          "import cycle found between modules %a",
          list(~sep=__arrow_sep, bold_str),
          cycles,
        )

      | UnresolvedModule(name) =>
        pf(ppf, "could not resolve module %a", bold_str, name)

      | FileNotFound(path) =>
        pf(ppf, "could not find file with path %a", bold_str, path)

      | ParseError(err, namespace, _) =>
        pf(
          ppf,
          "error found while parsing %a: %a",
          bold(Namespace.pp),
          namespace,
          pp_parse_err,
          err,
        )

      | InvalidModule(namespace) =>
        pf(ppf, "failed to parse module %a", bold(Namespace.pp), namespace)
    );

let pp_dump_compile_err: Fmt.t(compile_err) =
  Fmt.(
    ppf =>
      fun
      | ImportCycle(cycles) =>
        pf(ppf, "ImportCycle<%a>", list(~sep=__arrow_sep, string), cycles)
      | UnresolvedModule(name) => pf(ppf, "UnresolvedModule<%s>", name)
      | FileNotFound(path) => pf(ppf, "FileNotFound<%s>", path)
      | InvalidModule(namespace) =>
        pf(ppf, "InvalidModule<%a>", Namespace.pp, namespace)
      | ParseError(err, namespace, range) =>
        pf(
          ppf,
          "ParseError<%a, %a, %a>",
          pp_dump_parse_err,
          err,
          Namespace.pp,
          namespace,
          Range.pp,
          range,
        )
  );

let pp_dump_err_list: Fmt.t(list(compile_err)) =
  Fmt.(list(~sep=Sep.comma, pp_dump_compile_err));

let pp_err_list: Fmt.t(list(compile_err)) =
  ppf =>
    Fmt.(
      pf(
        ppf,
        "found some errors during compilation:@.@,%a",
        block(~layout=Vertical, ~sep=Sep.double_newline, pp_compile_err),
      )
    );
