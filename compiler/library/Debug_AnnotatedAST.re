open Globals;
open AnnotatedAST;
open Debug_Util;

let with_ctx = (f, x) => x |~> f;

let rec print_a_ast = (~depth=0) =>
  (
    fun
    | A_Statements(stmts) =>
      List.fold_left(
        (acc, s) => acc ++ (s |~> print_a_ast(~depth=depth + 1)),
        "",
        stmts,
      )
      |> Printf.sprintf("STATEMENTS:\n↳%s\n")
    | A_Import(module_, imports) =>
      Printf.sprintf(
        "IMPORT %s FROM %s",
        Util.print_comma_separated(
          with_ctx(Debug_AST.print_import),
          imports,
        ),
        module_,
      )
    | A_Declaration(decl) => decl |~> print_a_decl
  )
  % Printf.sprintf("\n%s%s", Util.repeat("\t", depth))
and print_a_decl =
  fun
  | A_ConstDecl(name, expr) =>
    expr |~> print_a_expr |> Printf.sprintf("CONST %s = %s", name)
  | A_StateDecl(name, params, props) => {
      let params_str =
        Util.print_comma_separated(with_ctx(print_a_property), params);
      let props_str =
        Util.print_comma_separated(with_ctx(print_a_state_prop), props);

      Printf.sprintf("STATE (%s, [%s], [%s])", name, params_str, props_str);
    }
  | A_ViewDecl(name, super, mixins, params, exprs) =>
    Printf.sprintf(
      "VIEW %s%s%s = %s",
      name,
      Util.print_optional(Printf.sprintf(" extends %s"), super),
      print_a_mixins(mixins),
      print_ctxl_lambda(params, exprs),
    )
  | A_FunctionDecl(name, params, exprs) =>
    Printf.sprintf(
      "FUNCTION %s = %s",
      name,
      print_ctxl_lambda(params, exprs),
    )
  | A_StyleDecl(name, params, rule_sets) =>
    Printf.sprintf(
      "STYLE %s = ([%s]) -> [%s]",
      name,
      Util.print_comma_separated(with_ctx(print_a_property), params),
      Util.print_comma_separated(
        with_ctx(print_a_style_rule_set),
        rule_sets,
      ),
    )
and print_a_property = ((name, type_def, default_val)) =>
  Printf.sprintf(
    "%s%s%s",
    name,
    Util.print_optional(print_a_type_def, type_def),
    print_a_assign(default_val),
  )
and print_a_expr =
  fun
  | A_NumericLit(n) => string_of_int(n)
  | A_BooleanLit(b) => string_of_bool(b)
  | A_StringLit(s) => s
  | A_Reference(reference) =>
    reference |~> print_a_ref |> Printf.sprintf("reference(%s)")
  | A_JSX(jsx) => jsx |~> print_a_jsx
  | A_AddExpr(lhs, rhs) =>
    Printf.sprintf("(%s + %s)", lhs |~> print_a_expr, rhs |~> print_a_expr)
  | A_SubExpr(lhs, rhs) =>
    Printf.sprintf("(%s - %s)", lhs |~> print_a_expr, rhs |~> print_a_expr)
  | A_MulExpr(lhs, rhs) =>
    Printf.sprintf("(%s * %s)", lhs |~> print_a_expr, rhs |~> print_a_expr)
  | A_DivExpr(lhs, rhs) =>
    Printf.sprintf("(%s / %s)", lhs |~> print_a_expr, rhs |~> print_a_expr)
  | A_LTExpr(lhs, rhs) =>
    Printf.sprintf("(%s < %s)", lhs |~> print_a_expr, rhs |~> print_a_expr)
  | A_LTEExpr(lhs, rhs) =>
    Printf.sprintf("(%s <= %s)", lhs |~> print_a_expr, rhs |~> print_a_expr)
  | A_GTExpr(lhs, rhs) =>
    Printf.sprintf("(%s > %s)", lhs |~> print_a_expr, rhs |~> print_a_expr)
  | A_GTEExpr(lhs, rhs) =>
    Printf.sprintf("(%s >= %s)", lhs |~> print_a_expr, rhs |~> print_a_expr)
  | A_AndExpr(lhs, rhs) =>
    Printf.sprintf("(%s && %s)", lhs |~> print_a_expr, rhs |~> print_a_expr)
  | A_OrExpr(lhs, rhs) =>
    Printf.sprintf("(%s || %s)", lhs |~> print_a_expr, rhs |~> print_a_expr)
and print_a_ref =
  fun
  | A_Variable(name) => Printf.sprintf("variable(%s)", name)
  | A_DotAccess(source, property) =>
    Printf.sprintf("%s.%s", source |~> print_a_ref, property)
  | A_Execution(source, exprs) =>
    Printf.sprintf(
      "exec %s(%s)",
      source |~> print_a_ref,
      Util.print_comma_separated(with_ctx(print_a_expr), exprs),
    )
and print_a_jsx =
  fun
  | A_Element(name, props, children) =>
    Printf.sprintf(
      "<%s%s>%s</%s>",
      name,
      Util.print_sequential(print_a_jsx_prop % Printf.sprintf(" %s"), props),
      Util.print_sequential(with_ctx(print_a_jsx), children),
      name,
    )
  | A_Fragment(children) =>
    Printf.sprintf(
      "<>%s</>",
      Util.print_sequential(with_ctx(print_a_jsx), children),
    )
  | A_TextNode(s) => s
  | A_EvalNode(expr) => expr |~> print_a_expr
and print_a_jsx_prop = ((name, expr)) =>
  Printf.sprintf("%s={%s}", name, expr |~> print_a_expr)
and print_a_state_prop =
  fun
  | A_Property((name, type_def, default_val)) =>
    print_a_property((name, type_def, default_val))
    |> Printf.sprintf("prop(%s)")
  | A_Getter(name, params, exprs) =>
    print_ctxl_lambda(params, exprs)
    |> Printf.sprintf("getter(%s = %s)", name)
  | A_Mutator(name, params, exprs) =>
    print_ctxl_lambda(params, exprs)
    |> Printf.sprintf("mutator(%s = %s)", name)
and print_a_mixins = mixins =>
  Util.print_comma_separated(x => x, mixins)
  |> (
    fun
    | "" => ""
    | _ as res => Printf.sprintf(" mixes %s", res)
  )
and print_a_style_rule = ((name, value)) =>
  Printf.sprintf("rule(%s = %s)", name |~> print_a_ref, value |~> print_a_ref)
and print_a_style_rule_set = ((key, rules)) =>
  Util.print_comma_separated(with_ctx(print_a_style_rule), rules)
  |> Printf.sprintf("ruleset(%s, [%s])", key |~> Debug_AST.print_style_key)
and print_a_type_def = type_def =>
  (
    switch (type_def) {
    | Boolean_t => "boolean"
    | Number_t => "number"
    | String_t => "string"
    | Array_t(typ) => print_a_type_def(typ) |> Printf.sprintf("%s[]")
    | Object_t(props) =>
      /* let rec next = s => switch() */
      Hashtbl.fold(
        (key, value, acc) =>
          acc
          ++ Printf.sprintf(
               "%s%s",
               String.length(acc) == 0 ? "" : ",",
               print_a_type_def(value),
             ),
        props,
        "",
      )
      |> Printf.sprintf("{ %s }")
    | _ => ""
    }
  )
  |> Printf.sprintf(": %s")
and print_a_assign = x =>
  Util.print_optional(with_ctx(print_a_expr % Printf.sprintf(" = %s")), x)
and print_ctxl_lambda = (params, exprs) => {
  let params_str =
    Util.print_comma_separated(with_ctx(print_a_property), params);
  let exprs_str = Util.print_comma_separated(with_ctx(print_a_expr), exprs);

  Printf.sprintf("([%s]) -> [%s]", params_str, exprs_str);
};
