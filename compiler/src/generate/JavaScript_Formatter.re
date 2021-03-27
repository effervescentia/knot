open Kore;
open JavaScript_AST;

let _import_variable_name =
  String.replace('.', '_') % String.replace('/', '$');

let fmt_string = String.escaped % Print.fmt("\"%s\"") % Pretty.string;

let rec fmt_expression = (module_type: Target.module_t) =>
  fun
  | Null => "null" |> Pretty.string
  | Boolean(x) => x |> string_of_bool |> Pretty.string
  | Number(x) => x |> Pretty.string
  | String(x) => x |> fmt_string
  | Group(Group(_) as x) => x |> fmt_expression(module_type)
  | Group(x) =>
    [
      "(" |> Pretty.string,
      x |> fmt_expression(module_type),
      ")" |> Pretty.string,
    ]
    |> Pretty.concat
  | Identifier(x) => x |> Pretty.string
  | DotAccess(expr, name) =>
    [
      expr |> fmt_expression(module_type),
      "." |> Pretty.string,
      name |> Pretty.string,
    ]
    |> Pretty.concat
  | FunctionCall(expr, args) =>
    [
      expr |> fmt_expression(module_type),
      "(" |> Pretty.string,
      args
      |> List.map(fmt_expression(module_type))
      |> List.intersperse(", " |> Pretty.string)
      |> Pretty.concat,
      ")" |> Pretty.string,
    ]
    |> Pretty.concat
  | UnaryOp(op, x) =>
    [op |> Pretty.string, x |> fmt_expression(module_type)] |> Pretty.concat
  | BinaryOp(op, l, r) =>
    [
      l |> fmt_expression(module_type),
      " " |> Pretty.string,
      op |> Pretty.string,
      " " |> Pretty.string,
      r |> fmt_expression(module_type),
    ]
    |> Pretty.concat
  | Ternary(x, y, z) =>
    [
      x |> fmt_expression(module_type),
      " ? " |> Pretty.string,
      y |> fmt_expression(module_type),
      " : " |> Pretty.string,
      z |> fmt_expression(module_type),
    ]
    |> Pretty.concat
  | Function(name, args, stmts) =>
    [
      "function" |> Pretty.string,
      switch (name) {
      | Some(name) => name |> Print.fmt(" %s") |> Pretty.string
      | None => Pretty.Nil
      },
      " (" |> Pretty.string,
      args
      |> List.map(Pretty.string)
      |> List.intersperse(", " |> Pretty.string)
      |> Pretty.concat,
      [") {" |> Pretty.string] |> Pretty.newline,
      stmts
      |> List.map(stmt =>
           [fmt_statement(module_type, stmt), ";" |> Pretty.string]
           |> Pretty.newline
         )
      |> Pretty.concat
      |> Pretty.indent(2),
      "}" |> Pretty.string,
    ]
    |> Pretty.concat
  | Object(props) =>
    props |> List.is_empty
      ? "{}" |> Pretty.string
      : [
          ["{" |> Pretty.string] |> Pretty.newline,
          props
          |> List.map(((name, expr)) =>
               [
                 name |> Pretty.string,
                 ": " |> Pretty.string,
                 expr |> fmt_expression(module_type),
               ]
               |> Pretty.concat
             )
          |> List.intersperse(["," |> Pretty.string] |> Pretty.newline)
          |> Pretty.concat
          |> Pretty.indent(2),
          [Pretty.Newline, "}" |> Pretty.string] |> Pretty.concat,
        ]
        |> Pretty.concat

and fmt_statement = (module_type: Target.module_t, stmt) =>
  switch (stmt) {
  | Expression(x) => [x |> fmt_expression(module_type)] |> Pretty.concat
  | Variable(name, expr) =>
    [
      "var " |> Pretty.string,
      name |> Pretty.string,
      " = " |> Pretty.string,
      expr |> fmt_expression(module_type),
    ]
    |> Pretty.concat
  | Assignment(lhs, rhs) =>
    [
      lhs |> fmt_expression(module_type),
      " = " |> Pretty.string,
      rhs |> fmt_expression(module_type),
    ]
    |> Pretty.concat
  | Return(expr) =>
    [
      "return" |> Pretty.string,
      switch (expr) {
      | Some(expr) =>
        [" " |> Pretty.string, expr |> fmt_expression(module_type)]
        |> Pretty.concat
      | None => Pretty.Nil
      },
    ]
    |> Pretty.concat
  | DefaultImport(namespace, id) =>
    switch (module_type) {
    | ES6 => fmt_es6_default_import(namespace, id)
    | Common => fmt_common_default_import(namespace, id)
    }
  | Import(namespace, imports) =>
    switch (module_type) {
    | ES6 => fmt_es6_named_imports(namespace, imports)
    | Common => fmt_common_named_imports(namespace, imports)
    }
  | Export(id) =>
    switch (module_type) {
    | ES6 => fmt_es6_export(id)
    | Common => fmt_common_export(id)
    }
  | EmptyExport =>
    switch (module_type) {
    | ES6 => ["export {}" |> Pretty.string] |> Pretty.concat
    | Common =>
      Assignment(DotAccess(Identifier("module"), "exports"), Object([]))
      |> fmt_statement(module_type)
    }
  }

and fmt_common_default_import = (namespace: string, id: string) =>
  Variable(id, FunctionCall(Identifier("require"), [String(namespace)]))
  |> fmt_statement(Target.Common)

and fmt_common_named_imports =
    (namespace: string, imports: list((string, option(string)))) =>
  (
    imports |> List.is_empty
      ? []
      : {
        let temp_variable_name =
          "$import$" ++ _import_variable_name(namespace);

        [
          Variable(
            temp_variable_name,
            FunctionCall(Identifier("require"), [String(namespace)]),
          ),
        ]
        @ (
          imports
          |> List.map(
               fun
               | (id, Some(label)) =>
                 Variable(
                   label,
                   DotAccess(Identifier(temp_variable_name), id),
                 )
               | (id, None) =>
                 Variable(
                   id,
                   DotAccess(Identifier(temp_variable_name), id),
                 ),
             )
        )
        @ [Assignment(Identifier(temp_variable_name), Null)]
        |> List.map(fmt_statement(Target.Common))
        |> List.intersperse([";" |> Pretty.string] |> Pretty.newline);
      }
  )
  |> Pretty.concat

and fmt_common_export = (name: string) =>
  Assignment(DotAccess(Identifier("exports"), name), Identifier(name))
  |> fmt_statement(Target.Common)

and fmt_es6_default_import = (namespace: string, id: string) =>
  [
    "import " |> Pretty.string,
    id |> Pretty.string,
    " from " |> Pretty.string,
    namespace |> fmt_string,
  ]
  |> Pretty.concat

and fmt_es6_named_imports =
    (namespace: string, imports: list((string, option(string)))) =>
  (
    imports |> List.is_empty
      ? []
      : [
        "import { " |> Pretty.string,
        imports
        |> List.map(
             fun
             | (id, Some(label)) =>
               [
                 id |> Pretty.string,
                 " as " |> Pretty.string,
                 label |> Pretty.string,
               ]
               |> Pretty.concat
             | (id, None) => id |> Pretty.string,
           )
        |> List.intersperse(", " |> Pretty.string)
        |> Pretty.concat,
        " } from " |> Pretty.string,
        namespace |> fmt_string,
      ]
  )
  |> Pretty.concat

and fmt_es6_export = (name: string) =>
  ["export { " |> Pretty.string, name |> Pretty.string, " }" |> Pretty.string]
  |> Pretty.concat;

let format = (module_type: Target.module_t, program: program_t): Pretty.t =>
  program
  |> List.map(stmt =>
       [fmt_statement(module_type, stmt), ";" |> Pretty.string]
       |> Pretty.newline
     )
  |> Pretty.concat;
