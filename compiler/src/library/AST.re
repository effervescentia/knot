open Infix;

type number_t =
  | Integer(int)
  | Float(float);

type primitive_t =
  | Nil
  | Boolean(bool)
  | Number(number_t);

type binary_operator_t =
  | Add
  | Subtract
  | Divide
  | Multiply;

type unary_operator_t =
  | Not
  | Negative;

type expression_t =
  | Primitive(primitive_t)
  | Identifier(string)
  | Group(expression_t)
  | BinaryOperation(binary_operator_t, expression_t, expression_t)
  | UnaryOperation(unary_operator_t, expression_t)
  | Closure(list(statement_t))
and statement_t =
  | Variable(string, expression_t)
  | Expression(expression_t)
  | EmptyStatement;

type declaration_t =
  | Constant(expression_t);

type module_statement_t =
  | Import(string, string)
  | Declaration(string, declaration_t)
  | EmptyModuleStatement;

type program_t = list(module_statement_t);

/* tag helpers */

let of_import = ((m_id, main)) => Import(m_id, main);
let of_decl = ((name, x)) => Declaration(name, x);
let of_const = x => Constant(x);
let of_prim = x => Primitive(x);
let nil = Nil;

/* print helpers */

let rec print_mod_stmt = stmt =>
  (
    switch (stmt) {
    | Import(m_id, main) => Print.fmt("import %s from \"%s\";", main, m_id)
    | Declaration(name, decl) => print_decl(name, decl)
    | EmptyModuleStatement => "\n"
    }
  )
  |> Print.fmt("%s\n")
and print_decl = name =>
  fun
  | Constant(expr) => Print.fmt("const %s = ;", name)
and print_binary_op =
  fun
  | Add => "+"
  | Subtract => "-"
  | Divide => "/"
  | Multiply => "*"
and print_unary_op =
  fun
  | Not => "!"
  | Negative => "-"
and print_num =
  fun
  | Integer(int) => string_of_int(int)
  | Float(float) => string_of_float(float)
and print_prim =
  fun
  | Nil => "nil"
  | Boolean(bool) => string_of_bool(bool)
  | Number(num) => print_num(num)
and print_expr =
  fun
  | Primitive(prim) => ""
  | Identifier(name) => name
  | Group(expr) => print_expr(expr) |> Print.fmt("(%s)")
  | BinaryOperation(op, lhs, rhs) =>
    Print.fmt(
      "%s %s %s",
      print_expr(lhs),
      print_binary_op(op),
      print_expr(rhs),
    )
  | UnaryOperation(op, expr) => print_unary_op(op) ++ print_expr(expr)
  | Closure(exprs) =>
    Print.many(~separator="\n", print_stmt, exprs) |> Print.fmt("{\n%s}")
and print_stmt = stmt =>
  (
    switch (stmt) {
    | Variable(name, expr) =>
      print_expr(expr) |> Print.fmt("let %s = %s", name)
    | Expression(expr) => print_expr(expr)
    | EmptyStatement => "\n"
    }
  )
  |> Print.fmt("%s;");
