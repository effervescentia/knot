open Reference;
open Infix;

module Fmt = Pretty.Formatters;

type type_table_t = Hashtbl.t(Export.t, Type.t);
type scope_tree_t = RangeTree.t(option(type_table_t));

type entry_t = {
  exports: type_table_t,
  ast: AST.program_t,
  scopes: scope_tree_t,
  raw: string,
};

type status_t =
  | Pending
  | Purged
  | Valid(entry_t)
  | Invalid(entry_t, list(Error.compile_err));

/**
 table for storing module ASTs
 */
type t = Hashtbl.t(Namespace.t, status_t);

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
let add =
    (
      id: Namespace.t,
      ast: AST.program_t,
      exports: list((Export.t, Type.t)),
      scopes: scope_tree_t,
      raw: string,
      table: t,
    ) =>
  Hashtbl.replace(
    table,
    id,
    Valid({
      ast,
      exports: exports |> List.to_seq |> Hashtbl.of_seq,
      scopes,
      raw,
    }),
  );

/**
 remove an entry from the table
 */
let remove = (id: Namespace.t, table: t) => Hashtbl.remove(table, id);

/**
 declare the type of an export member of an existing module
 */
let add_type =
    ((namespace, id): (Namespace.t, Export.t), value: 'a, table: t) =>
  switch (Hashtbl.find_opt(table, namespace)) {
  | Some(Valid(entry) | Invalid(entry, _)) =>
    Hashtbl.replace(entry.exports, id, value)

  | _ => ()
  };

let _compare_entry = (x, y) =>
  x.ast == y.ast && x.raw == y.raw && Hashtbl.compare(x.exports, y.exports);

/**
 compare two ModuleTables by direct equality
 */
let compare: (t, t) => bool =
  Hashtbl.compare(~compare=(x, y) =>
    switch (x, y) {
    | (Valid(x_entry), Valid(y_entry)) => _compare_entry(x_entry, y_entry)

    | (Invalid(x_entry, x_errors), Invalid(y_entry, y_errors)) =>
      _compare_entry(x_entry, y_entry) && x_errors == y_errors

    | _ => x == y
    }
  );

/* pretty printing */

let _pp_entry: Fmt.t(entry_t) =
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

let _pp_status: Fmt.t(status_t) =
  ppf =>
    fun
    | Valid(entry) => Fmt.pf(ppf, "Valid(%a)", _pp_entry, entry)
    | Invalid(entry, _) => Fmt.pf(ppf, "Invalid(%a)", _pp_entry, entry)
    | Purged => Fmt.pf(ppf, "Purged")
    | Pending => Fmt.pf(ppf, "Pending");

let pp: Fmt.t(t) =
  (ppf, table: t) =>
    Fmt.(table |> Hashtbl.pp(Namespace.pp, _pp_status, ppf));
