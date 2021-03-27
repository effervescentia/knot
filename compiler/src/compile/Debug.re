open Kore;

include Knot.Debug;

let print_module_table = (~debug=false, table: ModuleTable.t): string =>
  Hashtbl.to_seq_keys(table)
  |> List.of_seq
  |> Print.many(~separator="\n", key =>
       key
       |> Hashtbl.find(table)
       |> (
         ({ast, types}) =>
           ast
           |> (
             debug
               ? Debug.print_ast : Grammar.Formatter.format % Pretty.to_string
           )
           |> Print.fmt(
                "/* %s */\n\nexports: %s\n\n%s",
                Reference.Namespace.to_string(key),
                types
                |> Hashtbl.to_string(Functional.identity, Type.to_string)
                |> Pretty.to_string,
              )
       )
     );
