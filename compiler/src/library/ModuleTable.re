open Reference;

type scope_tree_t = RangeTree.t(option(Hashtbl.t(Export.t, Type.t)));

type entry_t = {
  types: Hashtbl.t(Export.t, Type.t),
  ast: AST.program_t,
  scopes: RangeTree.t(option(Hashtbl.t(Export.t, Type.t))),
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
    {ast, types: exports |> List.to_seq |> Hashtbl.of_seq, scopes, raw},
  );

/**
 remove an entry from the table
 */
let remove = (id: Namespace.t, table: t) => Hashtbl.remove(table, id);

/**
 declare the type of an export member of an existing module
 */
let add_type =
    ((namespace, id): (Namespace.t, Export.t), value: Type.t, table: t) =>
  if (Hashtbl.mem(table, namespace)) {
    let members = Hashtbl.find(table, namespace);

    Hashtbl.replace(members.types, id, value);
  };
