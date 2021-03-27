open Infix;
open AST;
open Reference;

let print_cursor =
  fun
  | Cursor.Point({line, column}) => Print.fmt("(%d:%d)", line, column)
  | Cursor.Range(start, end_) =>
    Print.fmt(
      "(%d:%d - %d:%d)",
      start.line,
      start.column,
      end_.line,
      end_.column,
    );

let print_ns = Namespace.to_string % Pretty.string;

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

let _print_attr = ((name, value)) =>
  [
    " " |> Pretty.string,
    name |> Pretty.string,
    "=\"" |> Pretty.string,
    value,
    "\"" |> Pretty.string,
  ]
  |> Pretty.concat;

let _print_entity = (~attrs=[], ~children=[], ~cursor=?, name) =>
  [
    [
      "<" |> Pretty.string,
      name |> Pretty.string,
      switch (cursor) {
      | Some(cursor) =>
        ["@" |> Pretty.string, cursor |> print_cursor |> Pretty.string]
        |> Pretty.concat
      | None => Pretty.Nil
      },
      attrs |> List.map(_print_attr) |> Pretty.concat,
    ]
    |> Pretty.concat,
    children |> List.is_empty
      ? " />" |> Pretty.string
      : [
          [">" |> Pretty.string] |> Pretty.newline,
          children
          |> List.map(child => [child] |> Pretty.newline)
          |> Pretty.concat
          |> Pretty.indent(2),
          ["</" |> Pretty.string, name |> Pretty.string, ">" |> Pretty.string]
          |> Pretty.concat,
        ]
        |> Pretty.concat,
  ]
  |> Pretty.concat;

let print_lexeme = (~attrs=[], name, value, cursor) =>
  _print_entity(~attrs, ~cursor, ~children=[value], name);

let print_typed_lexeme = (name, value, type_, cursor) =>
  print_lexeme(
    ~attrs=[
      ("type", [type_ |> Type.to_string |> Pretty.string] |> Pretty.concat),
    ],
    name,
    value,
    cursor,
  );

let rec print_expr =
  fun
  | Primitive((prim, type_, cursor)) =>
    print_typed_lexeme("Primitive", prim |> print_prim, type_, cursor)
  | Identifier((name, cursor)) =>
    print_lexeme("Identifier", name |> print_id, cursor)
  | JSX((jsx, cursor)) => print_lexeme("JSX", jsx |> print_jsx, cursor)
  | Group((expr, type_, cursor)) =>
    print_typed_lexeme("Group", expr |> print_expr, type_, cursor)
  | BinaryOp(op, (l_value, l_type, l_cursor), (r_value, r_type, r_cursor)) =>
    _print_entity(
      ~children=[
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
    _print_entity(~children=stmts |> List.map(print_stmt), "Closure")

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
        (
          "attrs",
          attrs
          |> List.map(((attr, cursor)) =>
               print_lexeme("Attribute", attr |> print_jsx_attr, cursor)
             )
          |> Pretty.concat,
        ),
        (
          "children",
          children
          |> List.map(((child, cursor)) =>
               print_lexeme("Child", child |> print_jsx_child, cursor)
             )
          |> Pretty.concat,
        ),
      ],
      ~children=[print_lexeme("Name", name |> print_id, cursor)],
      "Tag",
    )
  | Fragment(children) =>
    _print_entity(
      ~children=
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
        ~children=[
          name,
          print_typed_lexeme("Value", expr |> print_expr, type_, cursor),
        ],
        entity,
      )
    | (entity, name, None) => _print_entity(~children=[name], entity)
  )

and print_stmt = stmt =>
  switch (stmt) {
  | Variable((name, name_cursor), (expr, type_, cursor)) =>
    _print_entity(
      ~children=[
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
      ~children=[
        print_lexeme("Name", name |> print_id, name_cursor),
        print_typed_lexeme("Value", expr |> print_expr, type_, cursor),
      ],
      "Constant",
    )
  };

let print_mod_stmt =
  (
    fun
    | Import(namespace, imports) => [
        _print_entity(
          ~attrs=[
            ("namespace", namespace |> print_ns),
            ("main", namespace |> print_ns),
          ],
          ~children=
            imports
            |> List.map(
                 fun
                 | Main((name, cursor)) =>
                   _print_entity(
                     "Main",
                     ~children=[
                       print_lexeme("Name", name |> print_id, cursor),
                     ],
                   )
                 | Named((name, name_cursor), Some((label, label_cursor))) =>
                   _print_entity(
                     "Named",
                     ~children=[
                       print_lexeme("Name", name |> print_id, name_cursor),
                       print_lexeme("As", label |> print_id, label_cursor),
                     ],
                   )
                 | Named((name, cursor), None) =>
                   _print_entity(
                     "Named",
                     ~children=[
                       print_lexeme("Name", name |> print_id, cursor),
                     ],
                   ),
               ),
          "Import",
        ),
      ]
    | Declaration(name, decl) => [(name, decl) |> print_decl]
  )
  % Pretty.newline;

let print_ast = (program: program_t): string =>
  program |> List.map(print_mod_stmt) |> Pretty.concat |> Pretty.to_string;
