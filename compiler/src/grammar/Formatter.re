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
  | K_Invalid => "invalid";

let fmt_block = (print, x) =>
  Print.fmt(
    "[ type: %s, cursor: %s ] %s",
    Print.opt(fmt_type, Block.type_(x)),
    Cursor.to_string(Block.cursor(x)),
    print(Block.value(x)),
  );

let (<.>) = (x, f) => x |> Block.value |> f;

let rec fmt_mod_stmt = stmt =>
  (
    switch (stmt) {
    | Import(m_id, main) => Print.fmt("import %s from \"%s\";", main, m_id)
    | Declaration(name, decl) => fmt_decl((name, decl))
    | EmptyModuleStatement => "\n"
    }
  )
  |> Print.fmt("%s\n")
and fmt_decl = ((name, decl)) =>
  switch (decl) {
  | Constant(expr) => fmt_expr(expr) |> Print.fmt("const %s = %s;", name)
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
  | Integer(int) => Int64.to_string(int)
  | Float(float) => float |> Print.fmt("%f")
and fmt_prim =
  fun
  | Nil => "nil"
  | Boolean(bool) => string_of_bool(bool)
  | Number(num) => num <.> fmt_num
  | String(str) => str |> Print.fmt("\"%s\"")
and fmt_jsx =
  fun
  | Tag(name, attrs, children) =>
    List.length(children) == 0
      ? Print.many(fmt_jsx_attr % Print.fmt(" %s"), attrs)
        |> Print.fmt("<%s%s />", name)
      : Print.fmt(
          "<%s%s>%s</%s>",
          name,
          Print.many(fmt_jsx_attr % Print.fmt(" %s"), attrs),
          Print.many(~separator="\n", fmt_jsx_child, children),
          name,
        )
  | Fragment(_) => "<></>"
and fmt_jsx_child =
  fun
  | Node(jsx) => fmt_jsx(jsx)
  | Text(s) => s
  | InlineExpression(expr) => fmt_expr(expr) |> Print.fmt("{%s}")
and fmt_jsx_attr = attr =>
  (
    switch (attr) {
    | Class(name, value) => ("." ++ name, value)
    | ID(name, value) => ("#" ++ name, value)
    | Property(name, value) => (name, value)
    }
  )
  |> (
    fun
    | (name, Some(expr)) => fmt_expr(expr) |> Print.fmt("%s=%s", name)
    | (name, None) => name
  )
and fmt_expr =
  fun
  | Primitive(prim) => fmt_prim(prim)
  | Identifier(name) => name
  | JSX(jsx) => fmt_jsx(jsx)
  | Group(expr) => fmt_expr(expr) |> Print.fmt("(%s)")
  | BinaryOp(op, lhs, rhs) =>
    Print.fmt(
      "⟨%s %s %s⟩",
      fmt_expr(lhs),
      fmt_binary_op(op),
      fmt_expr(rhs),
    )
  | UnaryOp(op, expr) => fmt_unary_op(op) ++ fmt_expr(expr)
  | Closure(exprs) =>
    Print.many(~separator="\n", fmt_stmt, exprs) |> Print.fmt("{\n%s}")
and fmt_stmt = stmt =>
  (
    switch (stmt) {
    | Variable(name, expr) =>
      fmt_expr(expr) |> Print.fmt("let %s = %s", name)
    | Expression(expr) => fmt_expr(expr)
    | EmptyStatement => "\n"
    }
  )
  |> Print.fmt("%s;");

let format = (program: program_t): string =>
  Print.many(fmt_mod_stmt, program);
