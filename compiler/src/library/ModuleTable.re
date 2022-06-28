open Reference;
open Infix;

module Fmt = Pretty.Formatters;

type exports_t = list((Export.t, Type.t));
type type_table_t = Hashtbl.t(Export.t, Type.t);
type scope_tree_t = RangeTree.t(option(exports_t));

type library_t = {symbols: SymbolTable.t};

type module_t = {
  symbols: SymbolTable.t,
  ast: AST.program_t,
  scopes: scope_tree_t,
};

type entry_t =
  | Pending
  | Purged
  | Library(string, library_t)
  | Valid(string, module_t)
  | Partial(string, module_t, list(Error.compile_err))
  | Invalid(string, list(Error.compile_err));

/**
 table for storing module ASTs
 */
type t = Hashtbl.t(Namespace.t, entry_t);

/* static */

/**
 construct a new table for module information
 */
let create = (size: int): t => Hashtbl.create(size);

/* methods */

let find = (id: Namespace.t, table: t) => Hashtbl.find_opt(table, id);

/**
 add a module with associated export types and AST
 */
let add = (id: Namespace.t, entry: entry_t, table: t) =>
  Hashtbl.replace(table, id, entry);

/**
 remove an entry from the table
 */
let remove = (id: Namespace.t, table: t) => Hashtbl.remove(table, id);

/**
 purge an entry from the table
 */
let purge = (id: Namespace.t, table: t) =>
  Hashtbl.replace(table, id, Purged);

/**
 prepare a pending entry in the table
 */
let prepare = (id: Namespace.t, table: t) =>
  Hashtbl.replace(table, id, Pending);

/**
 unpack raw source code from an entry as an option
 */
let get_entry_raw: entry_t => option(string) =
  fun
  | Valid(raw, _) => Some(raw)
  /* | Invalid(Some(_) as data, _) => data */
  | _ => None;

/**
 unpack data from an entry as an option
 */
let get_entry_data: entry_t => option(module_t) =
  fun
  | Valid(_, data) => Some(data)
  | Partial(_, data, _) => Some(data)
  | _ => None;

let _compare_data = (x, y) => x.ast == y.ast && x.symbols == y.symbols;

/**
 compare two ModuleTables by direct equality
 */
let compare: (t, t) => bool =
  Hashtbl.compare(~compare=(x, y) =>
    switch (x, y) {
    | (Valid(x_raw, x_data), Valid(y_raw, y_data)) =>
      x_raw == y_raw && _compare_data(x_data, y_data)

    | (Partial(x_raw, x_data, x_errors), Partial(y_raw, y_data, y_errors)) =>
      x_raw == y_raw && _compare_data(x_data, y_data) && x_errors == y_errors

    | _ => x == y
    }
  );

/* pretty printing */

let _pp_library: Fmt.t(library_t) =
  (ppf, {symbols}) =>
    Fmt.(
      [("symbols", symbols |> ~@SymbolTable.pp)]
      |> List.to_seq
      |> Hashtbl.of_seq
      |> Hashtbl.pp(string, string, ppf)
    );

let _pp_module: Fmt.t(module_t) =
  (ppf, {ast, symbols}) =>
    Fmt.(
      [
        ("ast", ast |> ~@AST.Dump.pp),
        ("symbols", symbols |> ~@SymbolTable.pp),
      ]
      |> List.to_seq
      |> Hashtbl.of_seq
      |> Hashtbl.pp(string, string, ppf)
    );

let _pp_entry: Fmt.t(entry_t) =
  ppf =>
    fun
    | Library(raw, library) =>
      Fmt.pf(ppf, "Library(%s, %a)", raw, _pp_library, library)
    | Valid(raw, module_) =>
      Fmt.pf(ppf, "Valid(%s, %a)", raw, _pp_module, module_)
    | Partial(raw, data, errs) =>
      Fmt.(
        pf(
          ppf,
          "Partial(%s, %a, %a)",
          raw,
          _pp_module,
          data,
          Error.pp_dump_err_list,
          errs,
        )
      )
    | Invalid(raw, errs) =>
      Fmt.(pf(ppf, "Invalid(%s, %a)", raw, Error.pp_dump_err_list, errs))
    | Purged => Fmt.pf(ppf, "Purged")
    | Pending => Fmt.pf(ppf, "Pending");

let pp: Fmt.t(t) =
  (ppf, table: t) => Fmt.(table |> Hashtbl.pp(Namespace.pp, _pp_entry, ppf));
