open Core;

let rec repeat = (s, n) =>
  if (n == 0) {
    "";
  } else {
    s ++ repeat(s, n - 1);
  };

let print_comma_separated = (printer, xs) => {
  let rec next =
    fun
    | [x] => printer(x)
    | [x, ...xs] => printer(x) ++ "," ++ next(xs)
    | [] => "";

  next(xs);
};

let rec print_ast = (~depth=0) =>
  (
    fun
    | Statements(stmts) =>
      List.fold_left(
        (acc, s) => acc ++ print_ast(~depth=depth + 1, s),
        "",
        stmts,
      )
      |> Printf.sprintf("STATEMENTS:\n↳%s\n")
    | Import(module_, _) => Printf.sprintf("IMPORT FROM %s", module_)
    | Declaration(decl) =>
      print_decl(decl) |> Printf.sprintf("DECLARATION %s")
  )
  % Printf.sprintf("%s%s%s", depth == 0 ? "\n" : "", repeat("\t", depth))
and print_decl =
  fun
  | ConstDecl(name, expr) =>
    print_expr(expr) |> Printf.sprintf("const(%s, %s)", name)
  | StateDecl(name) => Printf.sprintf("state(%s)", name)
  | ViewDecl(name) => Printf.sprintf("view(%s)", name)
  | FunctionDecl(name, params, exprs) => {
      let params_str = print_comma_separated(print_param, params);
      let exprs_str = print_comma_separated(print_expr, exprs);

      Printf.sprintf("function(%s, [%s], [%s])", name, params_str, exprs_str);
    }
and print_param = ((name, type_def, default_val)) =>
  Printf.sprintf(
    "%s%s%s",
    name,
    switch (type_def) {
    | Some(s) => Printf.sprintf(": %s", s)
    | None => ""
    },
    switch (default_val) {
    | Some(s) => print_expr(s) |> Printf.sprintf(" = %s")
    | None => ""
    },
  )
and print_expr =
  fun
  | NumericLit(n) => string_of_int(n)
  | _ => "";