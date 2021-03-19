open Infix;
open AST;

let print_ns = AST.string_of_namespace;

let print_id =
  fun
  | Public(name) => name |> Pretty.string
  | Private(name) => Constants.private_prefix ++ name |> Pretty.string;

let print_type =
  Type.(
    fun
    | K_Invalid => "INVALID"
    | K_Unknown => "UNKNOWN"
    | K_Boolean => "bool"
    | K_Integer => "int"
    | K_Float => "float"
    | K_String => "string"
    | K_Element => "element"
    | K_Nil => "nil"
  )
  % Pretty.string;

let print_binary_op =
  fun
  | LogicalAnd => "LogicalAnd"
  | LogicalOr => "LogicalOr"
  | Add => "Add"
  | Subtract => "Subtract"
  | Divide => "Divide"
  | Multiply => "Multiply"
  | LessOrEqual => "LessOrEqual"
  | LessThan => "LessThan"
  | GreaterOrEqual => "GreaterOrEqual"
  | GreaterThan => "GreaterThan"
  | Equal => "Equal"
  | Unequal => "Unequal"
  | Exponent => "Exponent";

let print_unary_op =
  fun
  | Not => "Not"
  | Positive => "Positive"
  | Negative => "Negative";

let _print_attr = (name, value) =>
  [
    name |> Pretty.string,
    ":" |> Pretty.string,
    Newline,
    value |> Pretty.indent(2),
  ]
  |> Pretty.newline;

let _print_entity = (~attrs=[], ~cursor=?, name) =>
  [
    [
      "<" |> Pretty.string,
      name |> Pretty.string,
      switch (cursor) {
      | Some(cursor) =>
        ["@" |> Pretty.string, cursor |> Cursor.to_string |> Pretty.string]
        |> Pretty.concat
      | None => Pretty.Nil
      },
      ">" |> Pretty.string,
    ]
    |> Pretty.newline,
    attrs |> Pretty.concat |> Pretty.indent(2),
    ["</" |> Pretty.string, name |> Pretty.string, ">" |> Pretty.string]
    |> Pretty.concat,
  ]
  |> Pretty.newline;
let _print_lexeme = (~attrs=[], name, value, cursor) =>
  _print_entity(
    ~attrs=attrs @ [_print_attr("value", value)],
    ~cursor,
    name,
  );
let _print_typed_lexeme = (name, value, type_, cursor) =>
  _print_lexeme(
    ~attrs=[_print_attr("type", type_ |> print_type)],
    name,
    value,
    cursor,
  );

let rec print_expr =
  fun
  | Primitive((prim, type_, cursor)) =>
    _print_typed_lexeme("Primitive", prim |> print_prim, type_, cursor)
  | Identifier((name, cursor)) =>
    _print_lexeme("Identifier", name |> print_id, cursor)
  | JSX((jsx, cursor)) => _print_lexeme("JSX", jsx |> print_jsx, cursor)
  | Group((expr, type_, cursor)) =>
    _print_typed_lexeme("Group", expr |> print_expr, type_, cursor)
  | BinaryOp(op, (l_value, l_type, l_cursor), (r_value, r_type, r_cursor)) =>
    _print_entity(
      ~attrs=[
        _print_typed_lexeme(
          "LeftHandSide",
          l_value |> print_expr,
          l_type,
          l_cursor,
        ),
        _print_typed_lexeme(
          "RightHandSide",
          r_value |> print_expr,
          r_type,
          r_cursor,
        ),
      ],
      op |> print_binary_op,
    )
  | UnaryOp(op, (expr, type_, cursor)) =>
    _print_typed_lexeme(
      op |> print_unary_op,
      expr |> print_expr,
      type_,
      cursor,
    )
  | Closure(stmts) =>
    _print_entity(
      ~attrs=
        stmts |> List.map(stmt => [stmt |> print_stmt] |> Pretty.newline),
      "Closure",
    )

and print_prim =
  fun
  | Nil => "nil" |> Pretty.string
  | Boolean(bool) => bool |> string_of_bool |> Pretty.string
  | Number(num) => num |> print_num |> Pretty.string
  | String(str) => str |> Print.fmt("\"%s\"") |> Pretty.string

and print_num =
  fun
  | Integer(int) => int |> Int64.to_string
  | Float(float, precision) => float |> Print.fmt("%.*f", precision)

and print_jsx =
  fun
  | Tag((name, cursor), attrs, children) =>
    _print_entity(
      ~attrs=[
        _print_lexeme("Name", name |> print_id, cursor),
        _print_attr(
          "attrs",
          attrs
          |> List.map(((attr, cursor)) =>
               _print_lexeme("Attribute", attr |> print_jsx_attr, cursor)
             )
          |> Pretty.concat,
        ),
        _print_attr(
          "children",
          children
          |> List.map(((child, cursor)) =>
               _print_lexeme("Child", child |> print_jsx_child, cursor)
             )
          |> Pretty.concat,
        ),
      ],
      "Tag",
    )
  | Fragment(children) =>
    _print_entity(
      ~attrs=
        children
        |> List.map(((child, cursor)) =>
             _print_lexeme("Child", child |> print_jsx_child, cursor)
           ),
      "Fragment",
    )

and print_jsx_child =
  fun
  | Node((jsx, cursor)) => _print_lexeme("Node", jsx |> print_jsx, cursor)
  | Text(s) => s |> Pretty.string
  | InlineExpression((expr, type_, cursor)) =>
    _print_typed_lexeme("InlineExpression", expr |> print_expr, type_, cursor)

and print_jsx_attr = attr =>
  (
    switch (attr) {
    | Class((name, cursor), value) => (
        "Class",
        _print_lexeme("Name", name |> print_id, cursor),
        value,
      )
    | ID((name, cursor)) => (
        "ID",
        _print_lexeme("Name", name |> print_id, cursor),
        None,
      )
    | Property((name, cursor), value) => (
        "Property",
        _print_lexeme("Name", name |> print_id, cursor),
        value,
      )
    }
  )
  |> (
    fun
    | (entity, name, Some((expr, type_, cursor))) =>
      _print_entity(
        ~attrs=[
          name,
          _print_typed_lexeme("Value", expr |> print_expr, type_, cursor),
        ],
        entity,
      )
    | (entity, name, None) => _print_entity(~attrs=[name], entity)
  )

and print_stmt = stmt =>
  switch (stmt) {
  | Variable((name, name_cursor), (expr, type_, cursor)) =>
    _print_entity(
      ~attrs=[
        _print_lexeme("Name", name |> print_id, name_cursor),
        _print_typed_lexeme("Value", name |> print_id, type_, cursor),
      ],
      "Variable",
    )
  | Expression((expr, type_, cursor)) =>
    _print_typed_lexeme("Expression", expr |> print_expr, type_, cursor)
  };

let print_decl = (((name, name_cursor), decl)) =>
  switch (decl) {
  | Constant((expr, type_, cursor)) =>
    _print_entity(
      ~attrs=[
        _print_lexeme("Name", name |> print_id, name_cursor),
        _print_typed_lexeme("Value", expr |> print_expr, type_, cursor),
      ],
      "Constant",
    )
  };

let print_mod_stmt =
  (
    fun
    | Import(namespace, main) => [
        _print_entity(
          ~attrs=[
            _print_attr("namespace", namespace |> print_ns |> Pretty.string),
            _print_attr("main", namespace |> print_ns |> Pretty.string),
          ],
          "Import",
        ),
      ]
    | Declaration(name, decl) => [_print_entity(~attrs=[], "Declaration")]
  )
  % Pretty.newline;

let print_ast = (program: program_t): string =>
  program |> List.map(print_mod_stmt) |> Pretty.concat |> Pretty.to_string;
