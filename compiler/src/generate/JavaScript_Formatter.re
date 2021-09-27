open Kore;
open JavaScript_AST;
open Pretty;

let _import_variable_name =
  String.replace('.', '_') % String.replace('/', '$');

let fmt_string = String.escaped % Fmt.str("\"%s\"") % string;

let rec fmt_expression = (module_type: Target.module_t) =>
  fun
  | Null => string("null")
  | Boolean(x) => x |> string_of_bool |> string
  | Number(x) => string(x)
  | String(x) => fmt_string(x)
  | Group(Group(_) as x) => x |> fmt_expression(module_type)
  | Group(x) =>
    [string("("), x |> fmt_expression(module_type), string(")")] |> concat
  | Identifier(x) => string(x)
  | DotAccess(expr, name) =>
    [expr |> fmt_expression(module_type), string("."), string(name)]
    |> concat
  | FunctionCall(expr, args) =>
    [
      expr |> fmt_expression(module_type),
      string("("),
      args
      |> List.map(fmt_expression(module_type))
      |> List.intersperse(string(", "))
      |> concat,
      string(")"),
    ]
    |> concat
  | UnaryOp(op, x) =>
    [string(op), x |> fmt_expression(module_type)] |> concat
  | BinaryOp(op, l, r) =>
    [
      l |> fmt_expression(module_type),
      string(" "),
      string(op),
      string(" "),
      r |> fmt_expression(module_type),
    ]
    |> concat
  | Ternary(x, y, z) =>
    [
      x |> fmt_expression(module_type),
      string(" ? "),
      y |> fmt_expression(module_type),
      string(" : "),
      z |> fmt_expression(module_type),
    ]
    |> concat
  | Function(name, args, stmts) =>
    [
      string("function "),
      switch (name) {
      | Some(name) => string(name)
      | None => Nil
      },
      string("("),
      args |> List.map(string) |> List.intersperse(string(", ")) |> concat,
      [string(") {")] |> newline,
      stmts
      |> List.map(stmt =>
           [fmt_statement(module_type, stmt), string(";")] |> newline
         )
      |> concat
      |> indent(2),
      string("}"),
    ]
    |> concat
  | Object(props) =>
    List.is_empty(props)
      ? string("{}")
      : [
          [string("{")] |> newline,
          props
          |> List.map(((name, expr)) =>
               [
                 string(name),
                 string(": "),
                 expr |> fmt_expression(module_type),
               ]
               |> concat
             )
          |> List.intersperse([string(",")] |> newline)
          |> concat
          |> indent(2),
          [Newline, string("}")] |> concat,
        ]
        |> concat

and fmt_statement = (module_type: Target.module_t, stmt) =>
  switch (stmt) {
  | Expression(x) => [x |> fmt_expression(module_type)] |> concat
  | Variable(name, expr) =>
    [
      string("var "),
      string(name),
      string(" = "),
      expr |> fmt_expression(module_type),
    ]
    |> concat
  | Assignment(lhs, rhs) =>
    [
      lhs |> fmt_expression(module_type),
      string(" = "),
      rhs |> fmt_expression(module_type),
    ]
    |> concat
  | Return(expr) =>
    [
      string("return"),
      switch (expr) {
      | Some(expr) =>
        [string(" "), expr |> fmt_expression(module_type)] |> concat
      | None => Nil
      },
    ]
    |> concat
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
  | Export(id, alias) =>
    switch (module_type) {
    | ES6 => fmt_es6_export(~alias, id)
    | Common => fmt_common_export(~alias, id)
    }
  | EmptyExport =>
    switch (module_type) {
    | ES6 => [string("export {}")] |> concat
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
    List.is_empty(imports)
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
        |> List.intersperse([string(";")] |> newline);
      }
  )
  |> concat

and fmt_common_export = (~alias=None, name: string) =>
  Assignment(
    DotAccess(Identifier("exports"), alias |?: name),
    Identifier(name),
  )
  |> fmt_statement(Target.Common)

and fmt_es6_default_import = (namespace: string, id: string) =>
  [string("import "), string(id), string(" from "), fmt_string(namespace)]
  |> concat

and fmt_es6_named_imports =
    (namespace: string, imports: list((string, option(string)))) =>
  (
    List.is_empty(imports)
      ? []
      : [
        string("import { "),
        imports
        |> List.map(
             fun
             | (id, Some(label)) =>
               [string(id), string(" as "), string(label)] |> concat
             | (id, None) => string(id),
           )
        |> List.intersperse(string(", "))
        |> concat,
        string(" } from "),
        fmt_string(namespace),
      ]
  )
  |> concat

and fmt_es6_export = (~alias=None, name: string) =>
  [
    string("export { "),
    string(name),
    alias |?> Fmt.str(" as %s") % string |?: Nil,
    string(" }"),
  ]
  |> concat;

let format = (module_type: Target.module_t, program: program_t): Pretty.t =>
  program
  |> List.map(stmt =>
       [fmt_statement(module_type, stmt), string(";")] |> newline
     )
  |> concat;
