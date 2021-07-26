open Kore;
open AST;
open Reference;
open Pretty;

let __space = string(" ");
let __semicolon = string(";");
let __quotation_mark = string("\"");

let fmt_anon_id = id =>
  "'" ++ String.make(1, char_of_int(97 + id)) |> string;

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
  % string;

let fmt_unary_op =
  (
    fun
    | Not => "!"
    | Positive => "+"
    | Negative => "-"
  )
  % string;

let fmt_id = Identifier.to_string % string;

let fmt_num =
  (
    fun
    | Integer(int) => Int64.to_string(int)
    | Float(float, precision) => float |> Print.fmt("%.*f", precision)
  )
  % string;

let fmt_string = s =>
  [__quotation_mark, s |> String.escaped |> string, __quotation_mark]
  |> concat;

let fmt_ns = Namespace.to_string % fmt_string;

let fmt_prim =
  AST.(
    fun
    | Nil => string("nil")
    | Boolean(bool) => bool |> string_of_bool |> string
    | Number(num) => fmt_num(num)
    | String(str) => fmt_string(str)
  );

let rec fmt_jsx =
  fun
  | Tag(name, attrs, children) =>
    [
      string("<"),
      name |> Node.Raw.value |> fmt_id,
      List.is_empty(attrs)
        ? Nil
        : attrs
          |> List.map(
               Node.value % (attr => [__space, fmt_jsx_attr(attr)] |> concat),
             )
          |> concat,
      List.is_empty(children)
        ? string(" />")
        : [
            [string(">")] |> newline,
            children
            |> List.map(
                 Node.value % (child => [fmt_jsx_child(child)] |> newline),
               )
            |> concat
            |> indent(2),
            [string("</"), name |> Node.Raw.value |> fmt_id, string(">")]
            |> concat,
          ]
          |> concat,
    ]
    |> concat

  | Fragment(children) =>
    List.is_empty(children)
      ? string("<></>")
      : [
          [string("<>")] |> newline,
          children
          |> List.map(Node.value % fmt_jsx_child)
          |> concat
          |> indent(2),
          string("</>"),
        ]
        |> concat

and fmt_jsx_child =
  fun
  | Node(jsx) => jsx |> Node.value |> fmt_jsx
  | Text(s) => s |> Node.value |> string
  | InlineExpression(expr) =>
    [string("{"), expr |> Node.value |> fmt_expression, string("}")]
    |> concat

and fmt_jsx_attr = attr =>
  (
    switch (attr) {
    | Class(name, value) => (
        [string("."), name |> Node.Raw.value |> fmt_id] |> concat,
        value,
      )
    | ID(name) => (
        [string("#"), name |> Node.Raw.value |> fmt_id] |> concat,
        None,
      )
    | Property(name, value) => (name |> Node.Raw.value |> fmt_id, value)
    }
  )
  |> (
    fun
    | (name, Some(expr)) =>
      [name, string("="), expr |> Node.value |> fmt_jsx_attr_expr] |> concat

    | (name, None) => name
  )

and fmt_jsx_attr_expr = x =>
  switch (x) {
  | Primitive(_)
  | Identifier(_)
  | Group(_)
  | Closure(_)
  /* show tags or fragments with no children */
  | JSX((Tag(_, _, []) | Fragment([]), _, _)) => fmt_expression(x)
  | _ => [string("("), fmt_expression(x), string(")")] |> concat
  }

and fmt_expression =
  fun
  | Primitive(prim) => prim |> Node.value |> fmt_prim
  | Identifier(name) => name |> Node.value |> fmt_id
  | JSX(jsx) => jsx |> Node.value |> fmt_jsx
  /* collapse parentheses around unary values */
  | Group((
      (Primitive(_) | Identifier(_) | Group(_) | UnaryOp(_) | Closure(_)) as expr,
      _,
      _,
    )) =>
    fmt_expression(expr)
  | Group(expr) =>
    [string("("), expr |> Node.value |> fmt_expression, string(")")]
    |> concat
  | BinaryOp(op, lhs, rhs) =>
    [
      lhs |> Node.value |> fmt_expression,
      __space,
      fmt_binary_op(op),
      __space,
      rhs |> Node.value |> fmt_expression,
    ]
    |> concat
  | UnaryOp(op, expr) =>
    [fmt_unary_op(op), expr |> Node.value |> fmt_expression] |> concat
  | Closure(stmts) =>
    List.is_empty(stmts)
      ? string("{}")
      : [
          [string("{")] |> newline,
          stmts
          |> List.map(stmt => [fmt_statement(stmt)] |> newline)
          |> concat
          |> indent(2),
          string("}"),
        ]
        |> concat

and fmt_statement = stmt =>
  (
    switch (stmt) {
    | Variable(name, expr) => [
        string("let "),
        name |> Node.Raw.value |> fmt_id,
        string(" = "),
        expr |> Node.value |> fmt_expression,
      ]
    | Expression(expr) => [expr |> Node.value |> fmt_expression]
    }
  )
  @ [__semicolon]
  |> concat;

let fmt_declaration = ((name, decl)) =>
  (
    switch (decl) {
    | Constant(expr) => [
        string("const "),
        name |> Node.Raw.value |> fmt_id,
        string(" = "),
        expr |> Node.value |> fmt_expression,
        __semicolon,
      ]
    | Function(args, expr) => [
        string("func "),
        name |> Node.Raw.value |> fmt_id,
        List.is_empty(args)
          ? Nil
          : [
              string("("),
              args
              |> List.map(
                   Node.value
                   % (
                     ({name, default, type_}) =>
                       [
                         name |> Node.Raw.value |> fmt_id,
                         switch (default) {
                         | Some(expr) =>
                           [
                             string(" = "),
                             expr |> Node.value |> fmt_expression,
                           ]
                           |> concat
                         | None => Nil
                         },
                       ]
                       |> concat
                   ),
                 )
              |> List.intersperse(string(", "))
              |> concat,
              string(")"),
            ]
            |> concat,
        string(" -> "),
        expr |> Node.value |> fmt_expression,
        switch (Node.value(expr)) {
        | Closure(_) => Nil
        | _ => __semicolon
        },
      ]
    }
  )
  |> newline;

let fmt_imports = stmts => {
  let (internal_imports, external_imports) =
    stmts
    |> List.filter_map(
         fun
         | Import(namespace, imports) => Some((namespace, imports))
         | _ => None,
       )
    |> List.partition(
         Namespace.(
           fun
           | (Internal(_), _) => true
           | _ => false
         ),
       )
    |> Tuple.map2(
         List.sort((l, r) =>
           (l, r)
           |> Tuple.map2(
                fst
                % Namespace.(
                    fun
                    | Internal(name)
                    | External(name) => name
                  ),
              )
           |> Tuple.reduce2(String.compare)
         )
         % List.map(((namespace, imports)) => {
             let (main_import, named_imports) =
               imports
               |> List.fold_left(
                    ((m, n)) =>
                      fun
                      | MainImport(id) => (Some(Node.Raw.value(id)), n)
                      | NamedImport(id, label) => (
                          m,
                          [(Node.Raw.value(id), label), ...n],
                        ),
                    (None, []),
                  );

             [
               string("import"),
               main_import
               |> (
                 fun
                 | Some(id) =>
                   [string(" "), Identifier.to_string(id) |> string]
                   |> concat
                 | None => Nil
               ),
               switch (main_import, named_imports) {
               | (Some(_), [_, ..._]) => string(", ")
               | _ => Nil
               },
               List.is_empty(named_imports)
                 ? Nil
                 : [
                     string("{ "),
                     named_imports
                     |> List.sort((l, r) =>
                          (l, r)
                          |> Tuple.map2(fst % Identifier.to_string)
                          |> Tuple.reduce2(String.compare)
                        )
                     |> List.map(((id, label)) =>
                          [
                            fmt_id(id),
                            ...switch (label) {
                               | Some(label) => [
                                   string(" as "),
                                   label |> Node.Raw.value |> fmt_id,
                                 ]
                               | None => []
                               },
                          ]
                          |> concat
                        )
                     |> List.intersperse(string(", "))
                     |> concat,
                     string(" }"),
                   ]
                   |> concat,
               string(" from "),
               fmt_ns(namespace),
               __semicolon,
             ]
             |> newline;
           }),
       );

  external_imports
  @ (
    List.is_empty(external_imports) || List.is_empty(internal_imports)
      ? [] : [Newline]
  )
  @ internal_imports;
};

let fmt_declarations = stmts => {
  let declarations =
    stmts
    |> List.filter_map(
         fun
         | Declaration(MainExport(name) | NamedExport(name), decl) =>
           Some((name, decl))
         | _ => None,
       );

  let rec loop = (~acc=[]) =>
    fun
    | [] => List.rev(acc)
    /* do not add newline after the last statement */
    | [x] => loop(~acc=[fmt_declaration(x), ...acc], [])
    /* handle constant clustering logic */
    | [(_, Constant(_)) as x, ...xs] =>
      switch (xs) {
      /* no more statements, loop to return */
      | []
      /* followed by a constant, do not add newline */
      | [(_, Constant(_)), ..._] =>
        loop(~acc=[fmt_declaration(x), ...acc], xs)
      /* followed by other declarations, add a newline */
      | _ => loop(~acc=[fmt_declaration(x), Newline, ...acc], xs)
      }
    /* not a constant, add a newline */
    | [x, ...xs] => loop(~acc=[fmt_declaration(x), Newline, ...acc], xs);

  loop(declarations);
};

let format = (program: program_t): Pretty.t => {
  let imports = fmt_imports(program);
  let declarations = fmt_declarations(program);

  imports
  @ (
    List.is_empty(imports) || List.is_empty(declarations) ? [] : [Newline]
  )
  @ declarations
  |> concat;
};
