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
    {ast, exports: exports |> List.to_seq |> Hashtbl.of_seq, scopes, raw},
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
  if (Hashtbl.mem(table, namespace)) {
    let members = Hashtbl.find(table, namespace);

    Hashtbl.replace(members.exports, id, value);
  };

/**
 compare two ModuleTables by direct equality
 */
let compare: (t, t) => bool =
  Hashtbl.compare(~compare=(x, y) =>
    x.ast == y.ast && x.raw == y.raw && Hashtbl.compare(x.exports, y.exports)
  );

/* pretty printing */

let _pp_entry: Fmt.t(entry_t) =
  (ppf, {ast, raw, exports}) =>
    Fmt.(
      [
        ("ast", ast |> AST.Debug.print_ast |> Pretty2.to_string),
        ("exports", exports |> ~@Hashtbl.pp(Export.pp, Type.pp)),
        ("raw", raw),
      ]
      |> List.to_seq
      |> Hashtbl.of_seq
      |> Hashtbl.pp(string, string, ppf)
    );

let pp: Fmt.t(t) =
  (ppf, table: t) => Fmt.(table |> Hashtbl.pp(Namespace.pp, _pp_entry, ppf));
