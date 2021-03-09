open Kore;

type entry_t = {
  types: Hashtbl.t(string, Type.t),
  ast: AST.program_t,
};

/**
 table for storing module ASTs
 */
type t = Hashtbl.t(m_id, entry_t);

/* static */

/**
 construct a new table for module information
 */
let create = (size: int): t => Hashtbl.create(size);

/* methods */

/**
 add a module with associated export types and AST
 */
let add =
    (
      id: m_id,
      ast: AST.program_t,
      exports: list((string, Type.t)),
      table: t,
    ) =>
  Hashtbl.replace(
    table,
    id,
    {ast, types: exports |> List.to_seq |> Hashtbl.of_seq},
  );

/**
 remove an entry from the table
 */
let remove = (id: m_id, table: t) => Hashtbl.remove(table, id);

/**
 declare the type of an export member of an existing module
 */
let add_type = ((id, name): (m_id, string), value: Type.t, table: t) =>
  if (Hashtbl.mem(table, id)) {
    let members = Hashtbl.find(table, id);

    Hashtbl.replace(members.types, name, value);
  };

/**
 print the table as a string
 */
let to_string = (table: t): string =>
  Hashtbl.to_seq_keys(table)
  |> List.of_seq
  |> Print.many(~separator="\n", key =>
       Hashtbl.find(table, key).ast
       |> Grammar.Formatter.format
       |> Print.fmt("/* %s */\n\n%s", print_m_id(key))
     );
