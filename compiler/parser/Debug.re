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
    | [x, ...xs] => printer(x) ++ ", " ++ next(xs)
    | [] => "";

  next(xs);
};

let print_optional = printer =>
  fun
  | Some(x) => printer(x)
  | None => "";

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
    | Declaration(decl) => print_decl(decl)
  )
  % Printf.sprintf("\n%s%s", repeat("\t", depth))
and print_decl =
  fun
  | ConstDecl(name, expr) =>
    print_expr(expr) |> Printf.sprintf("CONST %s = %s", name)
  | StateDecl(name, params, props) => {
      let params_str = print_comma_separated(print_param, params);
      let props_str = print_comma_separated(print_state_prop, props);

      Printf.sprintf("STATE (%s, [%s], [%s])", name, params_str, props_str);
    }
  | ViewDecl(name, super, mixins, params, exprs) =>
    Printf.sprintf(
      "VIEW %s%s%s = %s",
      name,
      print_optional(Printf.sprintf(" extends %s"), super),
      print_comma_separated(x => x, mixins)
      |> (
        fun
        | "" => ""
        | _ as res => Printf.sprintf(" mixes %s", res)
      ),
      print_lambda(params, exprs),
    )
  | FunctionDecl(name, params, exprs) =>
    Printf.sprintf("FUNCTION %s = %s", name, print_lambda(params, exprs))
and print_param = ((name, type_def, default_val)) =>
  Printf.sprintf(
    "%s%s%s",
    name,
    print_type_def(type_def),
    print_assign(default_val),
  )
and print_expr =
  fun
  | NumericLit(n) => string_of_int(n)
  | BooleanLit(b) => string_of_bool(b)
  | StringLit(s) => s
  | Variable(name) => Printf.sprintf("variable(%s)", name)
  | AddExpr(lhs, rhs) =>
    Printf.sprintf("(%s + %s)", print_expr(lhs), print_expr(rhs))
  | SubExpr(lhs, rhs) =>
    Printf.sprintf("(%s - %s)", print_expr(lhs), print_expr(rhs))
  | MulExpr(lhs, rhs) =>
    Printf.sprintf("(%s * %s)", print_expr(lhs), print_expr(rhs))
  | DivExpr(lhs, rhs) =>
    Printf.sprintf("(%s / %s)", print_expr(lhs), print_expr(rhs))
  | LTExpr(lhs, rhs) =>
    Printf.sprintf("(%s < %s)", print_expr(lhs), print_expr(rhs))
  | LTEExpr(lhs, rhs) =>
    Printf.sprintf("(%s <= %s)", print_expr(lhs), print_expr(rhs))
  | GTExpr(lhs, rhs) =>
    Printf.sprintf("(%s > %s)", print_expr(lhs), print_expr(rhs))
  | GTEExpr(lhs, rhs) =>
    Printf.sprintf("(%s >= %s)", print_expr(lhs), print_expr(rhs))
  | AndExpr(lhs, rhs) =>
    Printf.sprintf("(%s && %s)", print_expr(lhs), print_expr(rhs))
  | OrExpr(lhs, rhs) =>
    Printf.sprintf("(%s || %s)", print_expr(lhs), print_expr(rhs))
and print_state_prop =
  fun
  | Property(name, type_def, default_val) =>
    print_param((name, type_def, default_val)) |> Printf.sprintf("prop(%s)")
  | Getter(name, params, exprs) =>
    print_lambda(params, exprs) |> Printf.sprintf("getter(%s = %s)", name)
  | Mutator(name, params, exprs) =>
    print_lambda(params, exprs) |> Printf.sprintf("mutator(%s = %s)", name)
and print_type_def = print_optional(Printf.sprintf(": %s"))
and print_assign = x =>
  print_optional(print_expr % Printf.sprintf(" = %s"), x)
and print_lambda = (params, exprs) => {
  let params_str = print_comma_separated(print_param, params);
  let exprs_str = print_comma_separated(print_expr, exprs);

  Printf.sprintf("([%s]) -> [%s]", params_str, exprs_str);
};