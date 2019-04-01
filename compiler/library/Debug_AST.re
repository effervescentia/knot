open Globals;
open AST;
open MemberType;
open Debug_Util;

let with_ctx = (f, x) => x |~> f;

let rec print_ast = (~depth=0) =>
  fun
  | Module(imports, stmts) =>
    (
      List.fold_left(
        (acc, s) => acc ++ print_module_import(~depth=depth + 1, s),
        "",
        imports,
      )
      |> Printf.sprintf("IMPORTS:\n↳%s\n")
    )
    ++ (
      List.fold_left(
        (acc, s) => acc ++ print_module_stmt(~depth=depth + 1, s),
        "",
        stmts,
      )
      |> Printf.sprintf("STATEMENTS:\n↳%s\n")
    )
and print_module_stmt = (~depth=0) =>
  (
    fun
    | Declaration(name, decl) => decl |~> print_decl(name)
    | Main(name, decl) =>
      decl |~> print_decl(name) |> Printf.sprintf("MAIN %s")
  )
  % Printf.sprintf("\n%s")
and print_module_import = (~depth=0) =>
  (
    fun
    | Import(module_, imports) =>
      Printf.sprintf(
        "IMPORT %s FROM %s",
        Util.print_comma_separated(with_ctx(print_import_target), imports),
        module_,
      )
  )
  % Printf.sprintf("\n%s")
and print_decl = name =>
  fun
  | ConstDecl(expr) =>
    expr |~> print_expr |> Printf.sprintf("CONST %s = %s", name)
  | StateDecl(params, props) => {
      let params_str =
        Util.print_comma_separated(with_ctx(print_property), params);
      let props_str =
        Util.print_comma_separated(
          ((name, prop)) => prop |~> print_state_member(name),
          props,
        );

      Printf.sprintf("STATE (%s, [%s], [%s])", name, params_str, props_str);
    }
  | ViewDecl(super, mixins, params, exprs) =>
    Printf.sprintf(
      "VIEW %s%s%s = %s",
      name,
      Util.print_optional(Printf.sprintf(" extends %s") |> with_ctx, super),
      print_mixins(mixins),
      print_lambda(params, exprs),
    )
  | FunctionDecl(params, exprs) =>
    Printf.sprintf("FUNCTION %s = %s", name, print_lambda(params, exprs))
  | StyleDecl(params, rule_sets) =>
    Printf.sprintf(
      "STYLE %s = ([%s]) -> [%s]",
      name,
      Util.print_comma_separated(with_ctx(print_property), params),
      Util.print_comma_separated(print_style_rule_set, rule_sets),
    )
and print_import_target =
  fun
  | MainExport(name) => Printf.sprintf("MAIN AS %s", name)
  | NamedExport(name, Some(new_name)) =>
    Printf.sprintf("%s AS %s", name, new_name)
  | NamedExport(name, None) => name
  | ModuleExport(name) => Printf.sprintf("* AS %s", name)
and print_property = ((name, type_def, default_val)) =>
  Printf.sprintf(
    "%s%s%s",
    name,
    print_type_def(type_def),
    print_assign(default_val),
  )
and print_scoped_expr =
  fun
  | ExpressionStatement(expr) => expr |~> print_expr
  | VariableDeclaration(name, expr) =>
    expr |~> print_expr |> Printf.sprintf("variable(%s = %s)", name)
  | VariableAssignment(refr, expr) =>
    Printf.sprintf("assign(%s = %s)", refr |~> print_ref, expr |~> print_expr)
and print_expr =
  fun
  | NumericLit(n) => string_of_int(n)
  | BooleanLit(b) => string_of_bool(b)
  | StringLit(s) => Printf.sprintf("\"%s\"", s)
  | FunctionLit(params, exprs) => print_lambda(params, exprs)
  | Reference(reference) =>
    reference |~> print_ref |> Printf.sprintf("reference(%s)")
  | JSX(jsx) => print_jsx(jsx)
  | EqualsExpr(lhs, rhs) =>
    Printf.sprintf("(%s == %s)", lhs |~> print_expr, rhs |~> print_expr)
  | AddExpr(lhs, rhs) =>
    Printf.sprintf("(%s + %s)", lhs |~> print_expr, rhs |~> print_expr)
  | SubExpr(lhs, rhs) =>
    Printf.sprintf("(%s - %s)", lhs |~> print_expr, rhs |~> print_expr)
  | MulExpr(lhs, rhs) =>
    Printf.sprintf("(%s * %s)", lhs |~> print_expr, rhs |~> print_expr)
  | DivExpr(lhs, rhs) =>
    Printf.sprintf("(%s / %s)", lhs |~> print_expr, rhs |~> print_expr)
  | LTExpr(lhs, rhs) =>
    Printf.sprintf("(%s < %s)", lhs |~> print_expr, rhs |~> print_expr)
  | LTEExpr(lhs, rhs) =>
    Printf.sprintf("(%s <= %s)", lhs |~> print_expr, rhs |~> print_expr)
  | GTExpr(lhs, rhs) =>
    Printf.sprintf("(%s > %s)", lhs |~> print_expr, rhs |~> print_expr)
  | GTEExpr(lhs, rhs) =>
    Printf.sprintf("(%s >= %s)", lhs |~> print_expr, rhs |~> print_expr)
  | AndExpr(lhs, rhs) =>
    Printf.sprintf("(%s && %s)", lhs |~> print_expr, rhs |~> print_expr)
  | OrExpr(lhs, rhs) =>
    Printf.sprintf("(%s || %s)", lhs |~> print_expr, rhs |~> print_expr)
  | TernaryExpr(predicate, if_expr, else_expr) =>
    Printf.sprintf(
      "(%s ? %s : %s)",
      predicate |~> print_expr,
      if_expr |~> print_expr,
      else_expr |~> print_expr,
    )
and print_ref =
  fun
  | Variable(name) => Printf.sprintf("variable(%s)", name)
  | SidecarVariable(name) => Printf.sprintf("sidecar_variable(%s)", name)
  | DotAccess(source, property) =>
    Printf.sprintf("%s.%s", source |~> print_ref, property)
  | Execution(source, exprs) =>
    Printf.sprintf(
      "exec %s(%s)",
      source |~> print_ref,
      Util.print_comma_separated(with_ctx(print_expr), exprs),
    )
and print_jsx =
  fun
  | Element(name, props, children) =>
    Printf.sprintf(
      "<%s%s>%s</%s>",
      name,
      Util.print_sequential(print_jsx_prop % Printf.sprintf(" %s"), props),
      Util.print_sequential(print_jsx, children),
      name,
    )
  | Fragment(children) =>
    Printf.sprintf("<>%s</>", Util.print_sequential(print_jsx, children))
  | TextNode(s) => s
  | EvalNode(expr) => expr |~> print_expr
and print_jsx_prop = ((name, expr)) =>
  Printf.sprintf("%s={%s}", name, expr |~> print_expr)
and print_state_member = name =>
  fun
  | `Property(type_def, default_val) as res =>
    print_state_property(name, res)
  | `Getter(params, exprs) as res
  | `Mutator(params, exprs) as res => print_state_method(name, res)
and print_state_property = name =>
  fun
  | `Property(type_def, default_val) =>
    print_property((name, type_def, default_val))
    |> Printf.sprintf("StateProperty(%s)")
and print_state_method = name =>
  fun
  | `Getter(params, exprs) =>
    print_lambda(params, exprs) |> Printf.sprintf("getter(%s = %s)", name)
  | `Mutator(params, exprs) =>
    print_lambda(params, exprs) |> Printf.sprintf("mutator(%s = %s)", name)
and print_mixins = mixins =>
  Util.print_comma_separated(with_ctx(x => x), mixins)
  |> (
    fun
    | "" => ""
    | _ as res => Printf.sprintf(" mixes %s", res)
  )
and print_style_rule = ((name, value)) =>
  Printf.sprintf("rule(%s = %s)", name |~> print_ref, value |~> print_ref)
and print_style_key =
  fun
  | ClassKey(name) => Printf.sprintf("class(%s)", name)
  | IdKey(name) => Printf.sprintf("id(%s)", name)
and print_style_rule_set = ((key, rules)) =>
  Util.print_comma_separated(print_style_rule, rules)
  |> Printf.sprintf("ruleset(%s, [%s])", print_style_key(key))
and print_type_def = Util.print_optional(fst % Printf.sprintf(": %s"))
and print_assign = x =>
  Util.print_optional(with_ctx(print_expr % Printf.sprintf(" = %s")), x)
and print_lambda = (params, exprs) => {
  let params_str =
    Util.print_comma_separated(with_ctx(print_property), params);
  let exprs_str =
    Util.print_comma_separated(with_ctx(print_scoped_expr), exprs);

  Printf.sprintf("([%s]) -> [%s]", params_str, exprs_str);
};
