open Reference;
open Infix;

module Fmt = Pretty.Formatters;

type type_table_t = Hashtbl.t(Export.t, Type.t);
type scope_tree_t = RangeTree.t(option(type_table_t));

type data_t = {
  exports: type_table_t,
  ast: AST.program_t,
  scopes: scope_tree_t,
  raw: string,
};

type entry_t =
  | Pending
  | Purged
  | Valid(data_t)
  | Invalid(option(data_t), list(Error.compile_err));

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
 declare the type of an export member of an existing module
 */
let add_type =
    ((namespace, id): (Namespace.t, Export.t), value: 'a, table: t) =>
  switch (Hashtbl.find_opt(table, namespace)) {
  | Some(Valid({exports}) | Invalid(Some({exports}), _)) =>
    Hashtbl.replace(exports, id, value)

  | _ => ()
  };

let _compare_data = (x, y) =>
  x.ast == y.ast && x.raw == y.raw && Hashtbl.compare(x.exports, y.exports);

/**
 compare two ModuleTables by direct equality
 */
let compare: (t, t) => bool =
  Hashtbl.compare(~compare=(x, y) =>
    switch (x, y) {
    | (Valid(x_data), Valid(y_data)) => _compare_data(x_data, y_data)

    | (Invalid(Some(x_data), x_errors), Invalid(Some(y_data), y_errors)) =>
      _compare_data(x_data, y_data) && x_errors == y_errors

    | _ => x == y
    }
  );

/**
 unpack data from an entry as an option
 */
let get_entry_data: entry_t => option(data_t) =
  fun
  | Valid(data) => Some(data)
  | Invalid(Some(_) as data, _) => data
  | _ => None;

/* pretty printing */

let _pp_data: Fmt.t(data_t) =
  (ppf, {ast, raw, exports}) =>
    Fmt.(
      [
        ("ast", ast |> ~@AST.Dump.pp),
        ("exports", exports |> ~@Hashtbl.pp(Export.pp, Type.pp)),
        ("raw", raw),
      ]
      |> List.to_seq
      |> Hashtbl.of_seq
      |> Hashtbl.pp(string, string, ppf)
    );

let _pp_entry: Fmt.t(entry_t) =
  ppf =>
    fun
    | Valid(data) => Fmt.pf(ppf, "Valid(%a)", _pp_data, data)
    | Invalid(data, errs) =>
      Fmt.(
        pf(
          ppf,
          "Invalid(%a, %a)",
          option(_pp_data),
          data,
          Error.pp_dump_err_list,
          errs,
        )
      )
    | Purged => Fmt.pf(ppf, "Purged")
    | Pending => Fmt.pf(ppf, "Pending");

let pp: Fmt.t(t) =
  (ppf, table: t) => Fmt.(table |> Hashtbl.pp(Namespace.pp, _pp_entry, ppf));
