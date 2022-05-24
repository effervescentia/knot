open Reference;
open Infix;

module Fmt = Pretty.Formatters;

type type_table_t = Hashtbl.t(Export.t, Type.t);
type scope_tree_t = RangeTree.t(option(type_table_t));

type data_t = {
  exports: type_table_t,
  ast: AST.program_t,
  scopes: scope_tree_t,
};

type entry_t =
  | Pending
  | Purged
  | Valid(string, data_t)
  | Partial(string, data_t, list(Error.compile_err))
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
let get_entry_data: entry_t => option(data_t) =
  fun
  | Valid(_, data) => Some(data)
  | Partial(_, data, _) => Some(data)
  | _ => None;

/**
 declare the type of an export member of an existing module
 */
let add_type =
    ((namespace, id): (Namespace.t, Export.t), value: 'a, table: t) =>
  Hashtbl.find_opt(table, namespace)
  |?< get_entry_data
  |> Option.iter(({exports}) => Hashtbl.replace(exports, id, value));

let _compare_data = (x, y) =>
  x.ast == y.ast && Hashtbl.compare(x.exports, y.exports);

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

let _pp_data: Fmt.t(data_t) =
  (ppf, {ast, exports}) =>
    Fmt.(
      [
        ("ast", ast |> ~@AST.Dump.pp),
        ("exports", exports |> ~@Hashtbl.pp(Export.pp, Type.pp)),
      ]
      |> List.to_seq
      |> Hashtbl.of_seq
      |> Hashtbl.pp(string, string, ppf)
    );

let _pp_entry: Fmt.t(entry_t) =
  ppf =>
    fun
    | Valid(raw, data) => Fmt.pf(ppf, "Valid(%s, %a)", raw, _pp_data, data)
    | Partial(raw, data, errs) =>
      Fmt.(
        pf(
          ppf,
          "Partial(%s, %a, %a)",
          raw,
          _pp_data,
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
