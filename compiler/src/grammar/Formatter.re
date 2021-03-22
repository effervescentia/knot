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
  | K_Element => "element"
  | K_Invalid => "invalid"
  | K_Unknown => "unknown";

let fmt_binary_op =
  (
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
  )
  % Pretty.string;

let fmt_unary_op =
  (
    fun
    | Not => "!"
    | Positive => "+"
    | Negative => "-"
  )
  % Pretty.string;

let fmt_id =
  (
    fun
    | Public(name) => name
    | Private(name) => Constants.private_prefix ++ name
  )
  % Pretty.string;

let fmt_ns = AST.string_of_namespace % Pretty.string;

let fmt_num =
  (
    fun
    | Integer(int) => int |> Int64.to_string
    | Float(float, precision) => float |> Print.fmt("%.*f", precision)
  )
  % Pretty.string;

let fmt_prim =
  fun
  | Nil => "nil" |> Pretty.string
  | Boolean(bool) => bool |> string_of_bool |> Pretty.string
  | Number(num) => num |> fmt_num
  | String(str) => str |> Print.fmt("\"%s\"") |> Pretty.string;

let rec fmt_jsx =
  fun
  | Tag((name, _), attrs, children) =>
    [
      "<" |> Pretty.string,
      name |> fmt_id,
      attrs |> List.is_empty
        ? Pretty.Nil
        : attrs
          |> List.map(
               fst
               % (
                 attr =>
                   [" " |> Pretty.string, attr |> fmt_jsx_attr]
                   |> Pretty.concat
               ),
             )
          |> Pretty.concat,
      children |> List.is_empty
        ? " />" |> Pretty.string
        : [
            [">" |> Pretty.string] |> Pretty.newline,
            children
            |> List.map(
                 fst % (child => [child |> fmt_jsx_child] |> Pretty.newline),
               )
            |> Pretty.concat
            |> Pretty.indent(2),
            ["</" |> Pretty.string, name |> fmt_id, ">" |> Pretty.string]
            |> Pretty.concat,
          ]
          |> Pretty.concat,
    ]
    |> Pretty.concat
  | Fragment(children) =>
    children |> List.is_empty
      ? "<></>" |> Pretty.string
      : [
          ["<>" |> Pretty.string] |> Pretty.newline,
          children
          |> List.map(fst % fmt_jsx_child)
          |> Pretty.concat
          |> Pretty.indent(2),
          "</>" |> Pretty.string,
        ]
        |> Pretty.concat

and fmt_jsx_child =
  fun
  | Node((jsx, _)) => jsx |> fmt_jsx
  | Text(s) => s |> Pretty.string
  | InlineExpression((expr, _, _)) =>
    ["{" |> Pretty.string, expr |> fmt_expr, "}" |> Pretty.string]
    |> Pretty.concat

and fmt_jsx_attr = attr =>
  (
    switch (attr) {
    | Class((name, _), value) => (
        ["." |> Pretty.string, name |> fmt_id] |> Pretty.concat,
        value,
      )
    | ID((name, _)) => (
        ["#" |> Pretty.string, name |> fmt_id] |> Pretty.concat,
        None,
      )
    | Property((name, _), value) => (name |> fmt_id, value)
    }
  )
  |> (
    fun
    | (name, Some((expr, _, _))) =>
      [name, "=" |> Pretty.string, expr |> fmt_expr] |> Pretty.concat

    | (name, None) => name
  )

and fmt_expr =
  fun
  | Primitive((prim, _, _)) => prim |> fmt_prim
  | Identifier((name, _)) => name |> fmt_id
  | JSX((jsx, _)) => jsx |> fmt_jsx
  | Group((expr, _, _)) =>
    ["(" |> Pretty.string, expr |> fmt_expr, ")" |> Pretty.string]
    |> Pretty.concat
  | BinaryOp(op, (lhs, _, _), (rhs, _, _)) =>
    [
      lhs |> fmt_expr,
      " " |> Pretty.string,
      op |> fmt_binary_op,
      " " |> Pretty.string,
      rhs |> fmt_expr,
    ]
    |> Pretty.concat
  | UnaryOp(op, (expr, _, _)) =>
    [op |> fmt_unary_op, expr |> fmt_expr] |> Pretty.concat
  | Closure(stmts) =>
    stmts |> List.is_empty
      ? "{}" |> Pretty.string
      : [
          ["{" |> Pretty.string] |> Pretty.newline,
          stmts
          |> List.map(stmt => [stmt |> fmt_stmt] |> Pretty.newline)
          |> Pretty.concat
          |> Pretty.indent(2),
          "}" |> Pretty.string,
        ]
        |> Pretty.concat

and fmt_stmt = stmt =>
  (
    switch (stmt) {
    | Variable((name, _), (expr, _, _)) => [
        "let " |> Pretty.string,
        name |> fmt_id,
        " = " |> Pretty.string,
        expr |> fmt_expr,
      ]
    | Expression((expr, _, _)) => [expr |> fmt_expr]
    }
  )
  @ [";" |> Pretty.string]
  |> Pretty.concat;

let fmt_decl = (((name, _), decl)) =>
  switch (decl) {
  | Constant((expr, _, _)) =>
    [
      "const " |> Pretty.string,
      name |> fmt_id,
      " = " |> Pretty.string,
      expr |> fmt_expr,
      ";" |> Pretty.string,
    ]
    |> Pretty.concat
  };

let fmt_mod_stmt = stmt =>
  (
    switch (stmt) {
    | Import(namespace, main) => [
        "import " |> Pretty.string,
        main |> Pretty.string,
        " from \"" |> Pretty.string,
        namespace |> fmt_ns,
        "\";" |> Pretty.string,
      ]

    | Declaration(name, decl) => [(name, decl) |> fmt_decl]
    }
  )
  |> Pretty.newline;

let format = (program: program_t): string =>
  program |> List.map(fmt_mod_stmt) |> Pretty.concat |> Pretty.to_string;
