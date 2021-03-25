open Kore;
open Reference;

type entry_t = {
  types: Hashtbl.t(string, Type.t),
  ast: AST.program_t,
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

/**
 add a module with associated export types and AST
 */
let add =
    (
      id: Namespace.t,
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
let remove = (id: Namespace.t, table: t) => Hashtbl.remove(table, id);

/**
 declare the type of an export member of an existing module
 */
let add_type = ((id, name): (Namespace.t, string), value: Type.t, table: t) =>
  if (Hashtbl.mem(table, id)) {
    let members = Hashtbl.find(table, id);

    Hashtbl.replace(members.types, name, value);
  };

/**
 print the table as a string
 */
let to_string = (~debug=false, table: t): string =>
  Hashtbl.to_seq_keys(table)
  |> List.of_seq
  |> Print.many(~separator="\n", key =>
       key
       |> Hashtbl.find(table)
       |> (
         ({ast, types}) =>
           ast
           |> (debug ? Debug.print_ast : Grammar.Formatter.format)
           |> Print.fmt(
                "/* %s */\n\nexports: %s\n\n%s",
                Namespace.to_string(key),
                types
                |> Hashtbl.to_string(Functional.identity, Type.to_string)
                |> Pretty.to_string,
              )
       )
     );
