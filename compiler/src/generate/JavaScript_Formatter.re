open Kore;
open JavaScript_AST;

let __import_prefix = "$import$";

let _import_variable_name =
  String.replace('.', '_') % String.replace('/', '$');

let _object_sep = Fmt.Sep.(of_sep(~trail=Trail.newline, ","));

let fmt_string: Fmt.t(string) =
  ppf => String.escaped % Fmt.(quote(string, ppf));

let rec fmt_expression = (module_type: Target.module_t): Fmt.t(expression_t) =>
  ppf =>
    fun
    | Null => Fmt.string(ppf, "null")
    | Boolean(x) => Fmt.bool(ppf, x)
    | Number(x) => Fmt.string(ppf, x)
    | String(x) => fmt_string(ppf, x)
    | Group(Group(_) as x) => fmt_expression(module_type, ppf, x)
    | Group(x) => Fmt.pf(ppf, "(%a)", fmt_expression(module_type), x)
    | Identifier(x) => Fmt.string(ppf, x)
    | DotAccess(expr, name) =>
      Fmt.pf(ppf, "%a.%s", fmt_expression(module_type), expr, name)
    | FunctionCall(expr, args) =>
      Fmt.(
        pf(
          ppf,
          "%a(%a)",
          fmt_expression(module_type),
          expr,
          list(~sep=Sep.comma, fmt_expression(module_type)),
          args,
        )
      )
    | UnaryOp(op, x) =>
      Fmt.pf(ppf, "%s%a", op, fmt_expression(module_type), x)
    | BinaryOp(op, l, r) =>
      Fmt.(
        pf(
          ppf,
          "%a %s %a",
          fmt_expression(module_type),
          l,
          op,
          fmt_expression(module_type),
          r,
        )
      )
    | Ternary(x, y, z) =>
      Fmt.(
        pf(
          ppf,
          "%a ? %a : %a",
          fmt_expression(module_type),
          x,
          fmt_expression(module_type),
          y,
          fmt_expression(module_type),
          z,
        )
      )
    | Function(name, args, stmts) =>
      Fmt.(
        pf(
          ppf,
          "@[<v>function %a(%a) %a@]",
          option(string),
          name,
          list(~sep=Sep.comma, string),
          args,
          closure(ppf => pf(ppf, "%a;", fmt_statement(module_type))),
          stmts,
        )
      )
    | Array([]) => Fmt.string(ppf, "[]")
    | Array(elements) =>
      Fmt.(
        pf(
          ppf,
          "@[<hv>[%a]@]",
          indented(list(~sep=_object_sep, fmt_expression(module_type))),
          elements,
        )
      )
    | Object([]) => Fmt.string(ppf, "{}")
    | Object(props) =>
      Fmt.(
        collection(
          ~layout=Vertical,
          ~sep=_object_sep,
          any("{"),
          any("}"),
          (ppf, (name, expr)) =>
            pf(
              ppf,
              "%a: %a",
              (ppf, x) =>
                String.is_first_alpha(x)
                  ? Fmt.string(ppf, x)
                  : Fmt.pf(ppf, "[\"%s\"]", String.escaped(x)),
              name,
              fmt_expression(module_type),
              expr,
            ),
          ppf,
          props,
        )
      )

and fmt_statement = (module_type: Target.module_t): Fmt.t(statement_t) =>
  ppf =>
    fun
    | Expression(x) => fmt_expression(module_type, ppf, x)
    | Variable(name, expr) =>
      Fmt.pf(ppf, "var %s = %a", name, fmt_expression(module_type), expr)
    | Assignment(lhs, rhs) =>
      Fmt.pf(
        ppf,
        "%a = %a",
        fmt_expression(module_type),
        lhs,
        fmt_expression(module_type),
        rhs,
      )
    | Return(expr) =>
      switch (expr) {
      | Some(expr) =>
        Fmt.pf(ppf, "return %a", fmt_expression(module_type), expr)
      | None => Fmt.string(ppf, "return")
      }
    | DefaultImport(namespace, id) =>
      switch (module_type) {
      | ES6 => fmt_es6_default_import(ppf, (namespace, id))
      | Common => fmt_common_default_import(ppf, (namespace, id))
      }
    | Import(namespace, imports) =>
      switch (module_type) {
      | ES6 => fmt_es6_named_imports(ppf, (namespace, imports))
      | Common => fmt_common_named_imports(ppf, (namespace, imports))
      }
    | Export(id, alias) =>
      switch (module_type) {
      | ES6 => fmt_es6_export(ppf, (id, alias))
      | Common => fmt_common_export(ppf, (id, alias))
      }
    | EmptyExport =>
      switch (module_type) {
      | ES6 => Fmt.string(ppf, "export {}")
      | Common =>
        Assignment(DotAccess(Identifier("module"), "exports"), Object([]))
        |> fmt_statement(module_type, ppf)
      }

and fmt_common_default_import: Fmt.t((string, string)) =
  (ppf, (namespace, id)) =>
    Variable(id, FunctionCall(Identifier("require"), [String(namespace)]))
    |> fmt_statement(Target.Common, ppf)

and fmt_common_named_imports:
  Fmt.t((string, list((string, option(string))))) =
  (ppf, (namespace, imports)) =>
    switch (imports) {
    | [] => Fmt.nop(ppf, ())
    | _ =>
      let temp_variable_name =
        __import_prefix ++ _import_variable_name(namespace);
      let statements =
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
        @ [Assignment(Identifier(temp_variable_name), Null)];

      statements
      |> Fmt.(
           list(
             ~layout=Vertical,
             ~sep=Sep.(of_sep(~trail=Trail.nop, ";")),
             fmt_statement(Target.Common),
             ppf,
           )
         );
    }

and fmt_common_export: Fmt.t((string, option(string))) =
  (ppf, (name, alias)) =>
    Assignment(
      DotAccess(Identifier("exports"), alias |?: name),
      Identifier(name),
    )
    |> fmt_statement(Target.Common, ppf)

and fmt_es6_default_import: Fmt.t((string, string)) =
  (ppf, (namespace, id)) =>
    Fmt.pf(ppf, "import %s from %a", id, fmt_string, namespace)

and _fmt_aliasable_entries: Fmt.t((string, option(string))) =
  ppf =>
    fun
    | (id, Some(label)) => Fmt.pf(ppf, "%s as %s", id, label)
    | (id, None) => Fmt.string(ppf, id)

and fmt_es6_named_imports: Fmt.t((string, list((string, option(string))))) =
  (ppf, (namespace, imports)) =>
    switch (imports) {
    | [] => Fmt.nop(ppf, ())
    | _ =>
      Fmt.(
        pf(
          ppf,
          "import { %a } from %a",
          list(~sep=Sep.comma, _fmt_aliasable_entries),
          imports,
          fmt_string,
          namespace,
        )
      )
    }

and fmt_es6_export: Fmt.t((string, option(string))) =
  (ppf, (name, alias)) =>
    Fmt.(pf(ppf, "export { %a }", _fmt_aliasable_entries, (name, alias)));

let format = (module_type: Target.module_t): Fmt.t(program_t) =>
  ppf =>
    Fmt.(
      page(
        list(~layout=Vertical, ~sep=Sep.newline, ppf =>
          pf(ppf, "%a;", fmt_statement(module_type))
        ),
        ppf,
      )
    );
