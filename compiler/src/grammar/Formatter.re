open Kore;
open AST;
open Type;
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
  | Tag((name, _), attrs, children) =>
    [
      string("<"),
      fmt_id(name),
      List.is_empty(attrs)
        ? Nil
        : attrs
          |> List.map(
               fst % (attr => [__space, fmt_jsx_attr(attr)] |> concat),
             )
          |> concat,
      List.is_empty(children)
        ? string(" />")
        : [
            [string(">")] |> newline,
            children
            |> List.map(fst % (child => [fmt_jsx_child(child)] |> newline))
            |> concat
            |> indent(2),
            [string("</"), fmt_id(name), string(">")] |> concat,
          ]
          |> concat,
    ]
    |> concat

  | Fragment(children) =>
    children |> List.is_empty
      ? string("<></>")
      : [
          [string("<>")] |> newline,
          children |> List.map(fst % fmt_jsx_child) |> concat |> indent(2),
          string("</>"),
        ]
        |> concat

and fmt_jsx_child =
  fun
  | Node((jsx, _)) => fmt_jsx(jsx)
  | Text((s, _)) => string(s)
  | InlineExpression((expr, _, _)) =>
    [string("{"), fmt_expression(expr), string("}")] |> concat

and fmt_jsx_attr = attr =>
  (
    switch (attr) {
    | Class((name, _), value) => (
        [string("."), fmt_id(name)] |> concat,
        value,
      )
    | ID((name, _)) => ([string("#"), fmt_id(name)] |> concat, None)
    | Property((name, _), value) => (fmt_id(name), value)
    }
  )
  |> (
    fun
    | (name, Some((expr, _, _))) =>
      [name, string("="), fmt_jsx_attr_expr(expr)] |> concat

    | (name, None) => name
  )

and fmt_jsx_attr_expr = x =>
  switch (x) {
  | Primitive(_)
  | Identifier(_)
  | Group(_)
  | Closure(_)
  /* show tags or fragments with no children */
  | JSX((Tag(_, _, []) | Fragment([]), _)) => fmt_expression(x)
  | _ => [string("("), fmt_expression(x), string(")")] |> concat
  }

and fmt_expression =
  fun
  | Primitive((prim, _, _)) => fmt_prim(prim)
  | Identifier((name, _)) => fmt_id(name)
  | JSX((jsx, _)) => fmt_jsx(jsx)
  /* collapse parentheses around unary values */
  | Group((
      (Primitive(_) | Identifier(_) | Group(_) | UnaryOp(_) | Closure(_)) as expr,
      _,
      _,
    )) =>
    expr |> fmt_expression
  | Group((expr, _, _)) =>
    [string("("), fmt_expression(expr), string(")")] |> concat
  | BinaryOp(op, (lhs, _, _), (rhs, _, _)) =>
    [
      lhs |> fmt_expression,
      __space,
      op |> fmt_binary_op,
      __space,
      rhs |> fmt_expression,
    ]
    |> concat
  | UnaryOp(op, (expr, _, _)) =>
    [fmt_unary_op(op), fmt_expression(expr)] |> concat
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
    | Variable((name, _), (expr, _, _)) => [
        string("let "),
        fmt_id(name),
        string(" = "),
        fmt_expression(expr),
      ]
    | Expression((expr, _, _)) => [fmt_expression(expr)]
    }
  )
  @ [__semicolon]
  |> concat;

let fmt_declaration = (((name, _), decl)) =>
  (
    switch (decl) {
    | Constant((expr, _, _)) => [
        string("const "),
        fmt_id(name),
        string(" = "),
        fmt_expression(expr),
        __semicolon,
      ]
    | Function(args, (expr, _, _)) => [
        string("func "),
        fmt_id(name),
        List.is_empty(args)
          ? Nil
          : [
              string("("),
              args
              |> List.map((({name, default}, type_)) =>
                   [
                     name |> fst |> fmt_id,
                     switch (default) {
                     | Some((expr, _, _)) =>
                       [string(" = "), fmt_expression(expr)] |> concat
                     | None => Nil
                     },
                   ]
                   |> concat
                 )
              |> List.intersperse(string(", "))
              |> concat,
              string(")"),
            ]
            |> concat,
        string(" -> "),
        fmt_expression(expr),
        switch (expr) {
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
                      | MainImport((id, _)) => (Some(id), n)
                      | NamedImport((id, _), label) => (
                          m,
                          [(id, label), ...n],
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
                               | Some((label, _)) => [
                                   string(" as "),
                                   fmt_id(label),
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
    | [x] => loop(~acc=[x |> fmt_declaration, ...acc], [])
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
    | [x, ...xs] => loop(~acc=[x |> fmt_declaration, Newline, ...acc], xs);

  loop(declarations);
};

let format = (program: program_t): Pretty.t => {
  let imports = program |> fmt_imports;
  let declarations = program |> fmt_declarations;

  imports
  @ (
    imports |> List.is_empty || declarations |> List.is_empty ? [] : [Newline]
  )
  @ declarations
  |> concat;
};
