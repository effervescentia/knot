open Infix;
open AST;
open Reference;

let print_ns = Namespace.to_string;

let print_id = Identifier.to_string % Pretty.string;

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
  |> Pretty.concat;

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

let print_lexeme = (~attrs=[], name, value, cursor) =>
  _print_entity(
    ~attrs=attrs @ [_print_attr("value", value)],
    ~cursor,
    name,
  );

let print_typed_lexeme = (name, value, type_, cursor) =>
  print_lexeme(
    ~attrs=[
      _print_attr(
        "type",
        [type_ |> Type.to_string |> Pretty.string] |> Pretty.newline,
      ),
    ],
    name,
    value,
    cursor,
  );

let rec print_expr =
  fun
  | Primitive((prim, type_, cursor)) =>
    print_typed_lexeme(
      "Primitive",
      [prim |> print_prim] |> Pretty.newline,
      type_,
      cursor,
    )
  | Identifier((name, cursor)) =>
    print_lexeme("Identifier", [name |> print_id] |> Pretty.newline, cursor)
  | JSX((jsx, cursor)) => print_lexeme("JSX", jsx |> print_jsx, cursor)
  | Group((expr, type_, cursor)) =>
    print_typed_lexeme("Group", expr |> print_expr, type_, cursor)
  | BinaryOp(op, (l_value, l_type, l_cursor), (r_value, r_type, r_cursor)) =>
    _print_entity(
      ~attrs=[
        print_typed_lexeme(
          "LeftHandSide",
          l_value |> print_expr,
          l_type,
          l_cursor,
        ),
        print_typed_lexeme(
          "RightHandSide",
          r_value |> print_expr,
          r_type,
          r_cursor,
        ),
      ],
      op |> print_binary_op,
    )
  | UnaryOp(op, (expr, type_, cursor)) =>
    print_typed_lexeme(
      op |> print_unary_op,
      expr |> print_expr,
      type_,
      cursor,
    )
  | Closure(stmts) =>
    _print_entity(~attrs=stmts |> List.map(print_stmt), "Closure")

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
        print_lexeme("Name", name |> print_id, cursor),
        _print_attr(
          "attrs",
          attrs
          |> List.map(((attr, cursor)) =>
               print_lexeme("Attribute", attr |> print_jsx_attr, cursor)
             )
          |> Pretty.concat,
        ),
        _print_attr(
          "children",
          children
          |> List.map(((child, cursor)) =>
               print_lexeme("Child", child |> print_jsx_child, cursor)
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
             print_lexeme("Child", child |> print_jsx_child, cursor)
           ),
      "Fragment",
    )

and print_jsx_child =
  fun
  | Node((jsx, cursor)) => print_lexeme("Node", jsx |> print_jsx, cursor)
  | Text(s) => s |> Pretty.string
  | InlineExpression((expr, type_, cursor)) =>
    print_typed_lexeme("InlineExpression", expr |> print_expr, type_, cursor)

and print_jsx_attr = attr =>
  (
    switch (attr) {
    | Class((name, cursor), value) => (
        "Class",
        print_lexeme("Name", name |> print_id, cursor),
        value,
      )
    | ID((name, cursor)) => (
        "ID",
        print_lexeme("Name", name |> print_id, cursor),
        None,
      )
    | Property((name, cursor), value) => (
        "Property",
        print_lexeme("Name", name |> print_id, cursor),
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
          print_typed_lexeme("Value", expr |> print_expr, type_, cursor),
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
        print_lexeme("Name", name |> print_id, name_cursor),
        print_typed_lexeme("Value", expr |> print_expr, type_, cursor),
      ],
      "Variable",
    )
  | Expression((expr, type_, cursor)) =>
    print_typed_lexeme("Expression", expr |> print_expr, type_, cursor)
  };

let print_decl = (((name, name_cursor), decl)) =>
  switch (decl) {
  | Constant((expr, type_, cursor)) =>
    _print_entity(
      ~attrs=[
        print_lexeme("Name", name |> print_id, name_cursor),
        print_typed_lexeme("Value", expr |> print_expr, type_, cursor),
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
    | Declaration(name, decl) => [(name, decl) |> print_decl]
  )
  % Pretty.newline;

let print_ast = (program: program_t): string =>
  program |> List.map(print_mod_stmt) |> Pretty.concat |> Pretty.to_string;
