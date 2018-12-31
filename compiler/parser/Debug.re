open Knot.Globals;
open AST;

let rec print_ast = (~depth=0) =>
  (
    fun
    | Statements(ss) =>
      List.fold_left(
        (acc, s) => acc ++ print_ast(~depth=depth + 1, s),
        "",
        ss,
      )
    | Import(module_, _) => Printf.sprintf("IMPORT FROM %s", module_)
    | Declaration(_) => ""
  )
  % Printf.sprintf("\t%s");