open Core;

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
    | Declaration(decl) =>
      print_decl(decl) |> Printf.sprintf("DECLARATION %s")
  )
  % Printf.sprintf("%s%s%s", depth == 0 ? "\n" : "", repeat("\t", depth))
and print_decl =
  fun
  | ConstDecl(s) => Printf.sprintf("const(%s)", s)
  | StateDecl(s) => Printf.sprintf("state(%s)", s)
  | ViewDecl(s) => Printf.sprintf("view(%s)", s)
  | FunctionDecl(s) => Printf.sprintf("function(%s)", s);