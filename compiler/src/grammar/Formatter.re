open Kore;
open AST;
open Type;

let fmt_type =
  fun
  | K_Nil => "nil"
  | K_Boolean => "bool"
  | K_Integer => "int"
  | K_Float => "float"
  | K_String => "string"
  | K_Invalid => "invalid"
  | K_Unknown => "unknown";

let fmt_block = (print, x) =>
  Print.fmt(
    "[ type: %s, cursor: %s, errors: (%s) ] %s",
    Block.type_(x) |> fmt_type,
    Block.cursor(x) |> Cursor.to_string,
    Print.opt(print_errs, Block.errors(x)),
    print(Block.value(x)),
  );

let rec fmt_mod_stmt = stmt =>
  (
    switch (stmt) {
    | Import(namespace, main) =>
      Print.fmt("import %s from \"%s\";", main, fmt_ns(namespace))
    | Declaration(name, decl) => fmt_decl((name, decl))
    }
  )
  |> Print.fmt("%s\n")
and fmt_ns = AST.string_of_namespace
and fmt_decl = (((name, _), decl)) =>
  switch (decl) {
  | Constant((expr, _, _)) =>
    expr |> fmt_expr |> Print.fmt("const %s = %s;", name |> fmt_id)
  }
and fmt_binary_op =
  fun
  | LogicalAnd => "&&"
  | LogicalOr => "||"
  | Add => "+"
  | Subtract => "-"
  | Divide => "/"
  | Multiply => "*"
  | LessOrEqual => "<="
  | LessThan => "<"
  | GreaterOrEqual => ">="
  | GreaterThan => ">"
  | Equal => "=="
  | Unequal => "!="
  | Exponent => "^"
and fmt_unary_op =
  fun
  | Not => "!"
  | Positive => "+"
  | Negative => "-"
and fmt_num =
  fun
  | Integer(int) => int |> Int64.to_string
  | Float(float, precision) => float |> Print.fmt("%.*f", precision)
and fmt_prim =
  fun
  | Nil => "nil"
  | Boolean(bool) => bool |> string_of_bool
  | Number(num) => num |> fmt_num
  | String(str) => str |> Print.fmt("\"%s\"")
and fmt_jsx =
  fun
  | Tag((name, _), attrs, children) =>
    List.length(children) == 0
      ? Print.many(fst % fmt_jsx_attr % Print.fmt(" %s"), attrs)
        |> Print.fmt("<%s%s />", name |> fmt_id)
      : Print.fmt(
          "<%s%s>%s</%s>",
          name |> fmt_id,
          Print.many(fst % fmt_jsx_attr % Print.fmt(" %s"), attrs),
          Print.many(~separator="\n", fst % fmt_jsx_child, children),
          name |> fmt_id,
        )
  | Fragment(_) => "<></>"
and fmt_jsx_child =
  fun
  | Node((jsx, _)) => jsx |> fmt_jsx
  | Text(s) => s
  | InlineExpression((expr, _, _)) => expr |> fmt_expr |> Print.fmt("{%s}")
and fmt_jsx_attr = attr =>
  (
    switch (attr) {
    | Class((name, _), value) => ("." ++ (name |> fmt_id), value)
    | ID((name, _)) => ("#" ++ (name |> fmt_id), None)
    | Property((name, _), value) => (name |> fmt_id, value)
    }
  )
  |> (
    fun
    | (name, Some((expr, _, _))) =>
      expr |> fmt_expr |> Print.fmt("%s=%s", name)
    | (name, None) => name
  )
and fmt_id =
  fun
  | Public(name) => name
  | Private(name) => Constants.private_prefix ++ name
and fmt_expr =
  fun
  | Primitive((prim, _, _)) => prim |> fmt_prim
  | Identifier((name, _)) => name |> fmt_id
  | JSX((jsx, _)) => jsx |> fmt_jsx
  | Group((expr, _, _)) => expr |> fmt_expr |> Print.fmt("(%s)")
  | BinaryOp(op, (lhs, _, _), (rhs, _, _)) =>
    Print.fmt(
      "⟨%s %s %s⟩",
      lhs |> fmt_expr,
      op |> fmt_binary_op,
      rhs |> fmt_expr,
    )
  | UnaryOp(op, (expr, _, _)) => fmt_unary_op(op) ++ fmt_expr(expr)
  | Closure(exprs) =>
    exprs |> Print.many(~separator="\n", fmt_stmt) |> Print.fmt("{\n%s}")
and fmt_stmt = stmt =>
  (
    switch (stmt) {
    | Variable((name, _), (expr, _, _)) =>
      expr |> fmt_expr |> Print.fmt("let %s = %s", name |> fmt_id)
    | Expression((expr, _, _)) => expr |> fmt_expr
    }
  )
  |> Print.fmt("%s;");

let format = (program: program_t): string =>
  Print.many(fmt_mod_stmt, program);
