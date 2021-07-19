open Reference;

type scope_tree_t = RangeTree.t(option(Hashtbl.t(Export.t, Type.t)));

type entry_t = {
  types: Hashtbl.t(Export.t, Type.t),
  ast: AST.Final.program_t,
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
      ast: AST.Final.program_t,
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

let to_string =
    (~debug=false, print_ast: AST.Final.program_t => string, table: t): string =>
  Hashtbl.to_seq_keys(table)
  |> List.of_seq
  |> Print.many(~separator="\n", key =>
       key
       |> Hashtbl.find(table)
       |> (
         ({ast, raw, types}) =>
           ast
           |> print_ast
           |> Print.fmt(
                "/* %s */\n\nexports: %s\n\nraw: \n\"%s\"\n\n%s",
                Reference.Namespace.to_string(key),
                types
                |> Hashtbl.to_string(Export.to_string, Type.to_string)
                |> Pretty.to_string,
                raw,
              )
       )
     );
