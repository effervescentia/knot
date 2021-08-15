open Reference;
open Infix;

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
 print a string representation for debugging
 */
let to_string = (table: t): string =>
  table
  |> Hashtbl.map_values(({ast, raw, exports}) =>
       [
         ("ast", ast |> AST.Debug.print_ast),
         (
           "exports",
           exports
           |> Hashtbl.print(Export.to_string, Type.to_string % Pretty.string),
         ),
         ("raw", Pretty.string(raw)),
       ]
       |> List.to_seq
       |> Hashtbl.of_seq
       |> Hashtbl.print(Functional.identity, Functional.identity)
     )
  |> Hashtbl.to_string(Namespace.to_string, Pretty.to_string);
