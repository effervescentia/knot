open Globals;
open AST;

let rec print_ast = (~depth=0) =>
  (
    fun
    | Statements(stmts) =>
      List.fold_left(
        (acc, s) => acc ++ print_ast(~depth=depth + 1, s),
        "",
        stmts,
      )
      |> Printf.sprintf("STATEMENTS:\n↳%s\n")
    | Import(module_, imports) =>
      Printf.sprintf(
        "IMPORT %s FROM %s",
        Util.print_comma_separated(print_import, imports),
        module_,
      )
    | Declaration(decl) => print_decl(decl)
  )
  % Printf.sprintf("\n%s%s", Util.repeat("\t", depth))
and print_decl =
  fun
  | ConstDecl(name, expr) =>
    print_expr(expr) |> Printf.sprintf("CONST %s = %s", name)
  | StateDecl(name, params, props) => {
      let params_str = Util.print_comma_separated(print_property, params);
      let props_str = Util.print_comma_separated(print_state_prop, props);

      Printf.sprintf("STATE (%s, [%s], [%s])", name, params_str, props_str);
    }
  | ViewDecl(name, super, mixins, params, exprs) =>
    Printf.sprintf(
      "VIEW %s%s%s = %s",
      name,
      Util.print_optional(Printf.sprintf(" extends %s"), super),
      print_mixins(mixins),
      print_lambda(params, exprs),
    )
  | FunctionDecl(name, params, exprs) =>
    Printf.sprintf("FUNCTION %s = %s", name, print_lambda(params, exprs))
  | StyleDecl(name, params, rule_sets) =>
    Printf.sprintf(
      "STYLE %s = ([%s]) -> [%s]",
      name,
      Util.print_comma_separated(print_property, params),
      Util.print_comma_separated(print_style_rule_set, rule_sets),
    )
and print_import =
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
and print_expr =
  fun
  | NumericLit(n) => string_of_int(n)
  | BooleanLit(b) => string_of_bool(b)
  | StringLit(s) => s
  | Reference(reference) =>
    print_ref(reference) |> Printf.sprintf("reference(%s)")
  | JSX(jsx) => print_jsx(jsx)
  | AddExpr(lhs, rhs) =>
    Printf.sprintf("(%s + %s)", print_expr(lhs), print_expr(rhs))
  | SubExpr(lhs, rhs) =>
    Printf.sprintf("(%s - %s)", print_expr(lhs), print_expr(rhs))
  | MulExpr(lhs, rhs) =>
    Printf.sprintf("(%s * %s)", print_expr(lhs), print_expr(rhs))
  | DivExpr(lhs, rhs) =>
    Printf.sprintf("(%s / %s)", print_expr(lhs), print_expr(rhs))
  | LTExpr(lhs, rhs) =>
    Printf.sprintf("(%s < %s)", print_expr(lhs), print_expr(rhs))
  | LTEExpr(lhs, rhs) =>
    Printf.sprintf("(%s <= %s)", print_expr(lhs), print_expr(rhs))
  | GTExpr(lhs, rhs) =>
    Printf.sprintf("(%s > %s)", print_expr(lhs), print_expr(rhs))
  | GTEExpr(lhs, rhs) =>
    Printf.sprintf("(%s >= %s)", print_expr(lhs), print_expr(rhs))
  | AndExpr(lhs, rhs) =>
    Printf.sprintf("(%s && %s)", print_expr(lhs), print_expr(rhs))
  | OrExpr(lhs, rhs) =>
    Printf.sprintf("(%s || %s)", print_expr(lhs), print_expr(rhs))
and print_ref =
  fun
  | Variable(name) => Printf.sprintf("variable(%s)", name)
  | DotAccess(source, property) =>
    Printf.sprintf("%s.%s", print_ref(source), property)
  | Execution(source, exprs) =>
    Printf.sprintf(
      "exec %s(%s)",
      print_ref(source),
      Util.print_comma_separated(print_expr, exprs),
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
  | EvalNode(expr) => print_expr(expr)
and print_jsx_prop = ((name, expr)) =>
  Printf.sprintf("%s={%s}", name, print_expr(expr))
and print_state_prop =
  fun
  | Property((name, type_def, default_val)) =>
    print_property((name, type_def, default_val))
    |> Printf.sprintf("prop(%s)")
  | Getter(name, params, exprs) =>
    print_lambda(params, exprs) |> Printf.sprintf("getter(%s = %s)", name)
  | Mutator(name, params, exprs) =>
    print_lambda(params, exprs) |> Printf.sprintf("mutator(%s = %s)", name)
and print_mixins = mixins =>
  Util.print_comma_separated(x => x, mixins)
  |> (
    fun
    | "" => ""
    | _ as res => Printf.sprintf(" mixes %s", res)
  )
and print_style_rule = ((name, value)) =>
  Printf.sprintf("rule(%s = %s)", print_ref(name), print_ref(value))
and print_style_key =
  fun
  | ClassKey(name) => Printf.sprintf("class(%s)", name)
  | IdKey(name) => Printf.sprintf("id(%s)", name)
and print_style_rule_set = ((key, rules)) =>
  Util.print_comma_separated(print_style_rule, rules)
  |> Printf.sprintf("ruleset(%s, [%s])", print_style_key(key))
and print_type_def = Util.print_optional(Printf.sprintf(": %s"))
and print_assign = x =>
  Util.print_optional(print_expr % Printf.sprintf(" = %s"), x)
and print_lambda = (params, exprs) => {
  let params_str = Util.print_comma_separated(print_property, params);
  let exprs_str = Util.print_comma_separated(print_expr, exprs);

  Printf.sprintf("([%s]) -> [%s]", params_str, exprs_str);
};
