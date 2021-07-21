open Reference;

type scope_tree_t('a) = RangeTree.t(option(Hashtbl.t(Export.t, 'a)));

type entry_t('a) = {
  types: Hashtbl.t(Export.t, 'a),
  ast: AST.program_t,
  scopes: scope_tree_t('a),
  raw: string,
};

/**
 table for storing module ASTs
 */
type t('a) = Hashtbl.t(Namespace.t, entry_t('a));

/* static */

/**
 construct a new table for module information
 */
let create = (size: int): t('a) => Hashtbl.create(size);

/* methods */

let find = (id: Namespace.t, table: t('a)) => Hashtbl.find_opt(table, id);

/**
 add a module with associated export types and AST
 */
let add =
    (
      id: Namespace.t,
      ast: AST.program_t,
      exports: list((Export.t, 'a)),
      scopes: scope_tree_t('a),
      raw: string,
      table: t('a),
    ) =>
  Hashtbl.replace(
    table,
    id,
    {ast, types: exports |> List.to_seq |> Hashtbl.of_seq, scopes, raw},
  );

/**
 remove an entry from the table
 */
let remove = (id: Namespace.t, table: t('a)) => Hashtbl.remove(table, id);

/**
 declare the type of an export member of an existing module
 */
let add_type =
    ((namespace, id): (Namespace.t, Export.t), value: 'a, table: t('a)) =>
  if (Hashtbl.mem(table, namespace)) {
    let members = Hashtbl.find(table, namespace);

    Hashtbl.replace(members.types, id, value);
  };

/**
 print a string representation for debugging
 */
let to_string =
    (~debug=false, type_to_string: 'a => string, table: t('a)): string =>
  Hashtbl.to_seq_keys(table)
  |> List.of_seq
  |> Print.many(~separator="\n", key =>
       key
       |> Hashtbl.find(table)
       |> (
         ({ast, raw, types}) =>
           ast
           |> Debug.print_ast
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
