open Knot.Globals;
open AST;

let rec repeat = (s, n) =>
  if (n == 0) {
    "";
  } else {
    s ++ repeat(s, n - 1);
  };

let rec print_ast = (~depth=0) =>
  (
    fun
    | Statements(ss) =>
      List.fold_left(
        (acc, s) => acc ++ print_ast(~depth=depth + 1, s),
        "",
        ss,
      )
      |> Printf.sprintf("STATEMENTS:\n↳%s\n")
    | Import(module_, _) => Printf.sprintf("IMPORT FROM %s", module_)
    | Declaration(_) => ""
  )
  % Printf.sprintf("%s%s", repeat("\t", depth));