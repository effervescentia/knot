open Kore;
open AST;
open Type;
open Reference;

let __space = " " |> Pretty.string;
let __semicolon = ";" |> Pretty.string;
let __quotation_mark = "\"" |> Pretty.string;

let fmt_anon_id = id =>
  "'" ++ String.make(1, char_of_int(97 + id)) |> Pretty.string;

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

let fmt_id = Identifier.to_string % Pretty.string;

let fmt_num =
  (
    fun
    | Integer(int) => int |> Int64.to_string
    | Float(float, precision) => float |> Print.fmt("%.*f", precision)
  )
  % Pretty.string;

let fmt_string = s =>
  [__quotation_mark, s |> String.escaped |> Pretty.string, __quotation_mark]
  |> Pretty.concat;

let fmt_ns = Namespace.to_string % fmt_string;

let fmt_prim =
  fun
  | Nil => "nil" |> Pretty.string
  | Boolean(bool) => bool |> string_of_bool |> Pretty.string
  | Number(num) => num |> fmt_num
  | String(str) => str |> fmt_string;

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
               % (attr => [__space, attr |> fmt_jsx_attr] |> Pretty.concat),
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
      [name, "=" |> Pretty.string, expr |> fmt_jsx_attr_expr] |> Pretty.concat

    | (name, None) => name
  )

and fmt_jsx_attr_expr = x =>
  switch (x) {
  | Primitive(_)
  | Identifier(_)
  | Group(_)
  | Closure(_)
  /* show tags or fragments with no children */
  | JSX((Tag(_, _, []) | Fragment([]), _)) => x |> fmt_expr
  | _ =>
    ["(" |> Pretty.string, x |> fmt_expr, ")" |> Pretty.string]
    |> Pretty.concat
  }

and fmt_expr =
  fun
  | Primitive((prim, _, _)) => prim |> fmt_prim
  | Identifier((name, _)) => name |> fmt_id
  | JSX((jsx, _)) => jsx |> fmt_jsx
  /* collapse parentheses around unary values */
  | Group((
      (Primitive(_) | Identifier(_) | Group(_) | UnaryOp(_) | Closure(_)) as expr,
      _,
      _,
    )) =>
    expr |> fmt_expr
  | Group((expr, _, _)) =>
    ["(" |> Pretty.string, expr |> fmt_expr, ")" |> Pretty.string]
    |> Pretty.concat
  | BinaryOp(op, (lhs, _, _), (rhs, _, _)) =>
    [lhs |> fmt_expr, __space, op |> fmt_binary_op, __space, rhs |> fmt_expr]
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
  @ [__semicolon]
  |> Pretty.concat;

let fmt_decl = (((name, _), decl)) =>
  (
    switch (decl) {
    | Constant((expr, _, _)) => [
        "const " |> Pretty.string,
        name |> fmt_id,
        " = " |> Pretty.string,
        expr |> fmt_expr,
        __semicolon,
      ]
    }
  )
  |> Pretty.newline;

let fmt_imports = stmts => {
  let (internal_imports, external_imports) =
    stmts
    |> List.filter_map(
         fun
         | Import(namespace, main) => Some((namespace, main))
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
         % List.map(((namespace, main)) =>
             [
               "import " |> Pretty.string,
               main |> Pretty.string,
               " from " |> Pretty.string,
               namespace |> fmt_ns,
               __semicolon,
             ]
             |> Pretty.newline
           ),
       );

  external_imports
  @ (
    external_imports |> List.is_empty || internal_imports |> List.is_empty
      ? [] : [Pretty.Newline]
  )
  @ internal_imports;
};

let fmt_declarations = stmts => {
  let declarations =
    stmts
    |> List.filter_map(
         fun
         | Declaration(name, decl) => Some((name, decl))
         | _ => None,
       );

  let rec loop = (~acc=[]) =>
    fun
    | [] => acc |> List.rev
    /* do not add newline after the last statement */
    | [x] => loop(~acc=[x |> fmt_decl, ...acc], [])
    /* handle constant clustering logic */
    | [(_, Constant(_)) as x, ...xs] =>
      switch (xs) {
      /* no more statements, loop to return */
      | []
      /* followed by a constant, do not add newline */
      | [(_, Constant(_)), ..._] => loop(~acc=[x |> fmt_decl, ...acc], xs)
      /* followed by other declarations, add a newline */
      | _ => loop(~acc=[x |> fmt_decl, Pretty.Newline, ...acc], xs)
      }
    /* not a constant, add a newline */
    | [x, ...xs] => loop(~acc=[x |> fmt_decl, Pretty.Newline, ...acc], xs);

  loop(declarations);
};

let format = (program: program_t): string => {
  let imports = program |> fmt_imports;
  let declarations = program |> fmt_declarations;

  imports
  @ (
    imports |> List.is_empty || declarations |> List.is_empty
      ? [] : [Pretty.Newline]
  )
  @ declarations
  |> Pretty.concat
  |> Pretty.to_string;
};
