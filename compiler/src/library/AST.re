open Infix;

type number_t =
  | Integer(Int64.t)
  | Float(float);

type primitive_t =
  | Nil
  | Boolean(bool)
  | Number(number_t)
  | String(string);

type binary_operator_t =
  /* logical operators */
  | LogicalAnd
  | LogicalOr
  /* comparative operators */
  | LessOrEqual
  | LessThan
  | GreaterOrEqual
  | GreaterThan
  /* equality operators */
  | Equal
  | Unequal
  /* arithmetic operators */
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
  | BinaryOp(binary_operator_t, expression_t, expression_t)
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
let of_var = ((name, x)) => Variable(name, x);
let of_expr = x => Expression(x);
let of_id = x => Identifier(x);
let of_group = x => Group(x);
let of_closure = xs => Closure(xs);

let of_and_op = ((l, r)) => BinaryOp(LogicalAnd, l, r);
let of_or_op = ((l, r)) => BinaryOp(LogicalOr, l, r);

let of_mult_op = ((l, r)) => BinaryOp(Multiply, l, r);
let of_div_op = ((l, r)) => BinaryOp(Divide, l, r);
let of_add_op = ((l, r)) => BinaryOp(Add, l, r);
let of_sub_op = ((l, r)) => BinaryOp(Subtract, l, r);

let of_lt_op = ((l, r)) => BinaryOp(LessThan, l, r);
let of_lte_op = ((l, r)) => BinaryOp(LessOrEqual, l, r);
let of_gt_op = ((l, r)) => BinaryOp(GreaterThan, l, r);
let of_gte_op = ((l, r)) => BinaryOp(GreaterOrEqual, l, r);

let of_eq_op = ((l, r)) => BinaryOp(Equal, l, r);
let of_ineq_op = ((l, r)) => BinaryOp(Unequal, l, r);

let of_prim = x => Primitive(x);
let of_bool = x => Boolean(x);
let of_int = x => Integer(x);
let of_float = x => Float(x);
let of_string = x => String(x);
let of_num = x => Number(x);
let nil = Nil;

/* print helpers */

let rec print_mod_stmt = stmt =>
  (
    switch (stmt) {
    | Import(m_id, main) => Print.fmt("import %s from \"%s\";", main, m_id)
    | Declaration(name, decl) => print_decl((name, decl))
    | EmptyModuleStatement => "\n"
    }
  )
  |> Print.fmt("%s\n")
and print_decl = ((name, decl)) =>
  switch (decl) {
  | Constant(expr) => print_expr(expr) |> Print.fmt("const %s = %s;", name)
  }
and print_binary_op =
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
and print_unary_op =
  fun
  | Not => "!"
  | Negative => "-"
and print_num =
  fun
  | Integer(int) => Int64.to_string(int)
  | Float(float) => float |> Print.fmt("%f")
and print_prim =
  fun
  | Nil => "nil"
  | Boolean(bool) => string_of_bool(bool)
  | Number(num) => print_num(num)
  | String(str) => str |> Print.fmt("\"%s\"")
and print_expr =
  fun
  | Primitive(prim) => print_prim(prim)
  | Identifier(name) => name
  | Group(expr) => print_expr(expr) |> Print.fmt("(%s)")
  | BinaryOp(op, lhs, rhs) =>
    Print.fmt(
      "(%s %s %s)",
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
