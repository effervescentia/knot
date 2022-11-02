open Knot.Kore;

let pp_declaration:
  Fmt.t(AST.TypeExpression.raw_t) =>
  Fmt.t((string, AST.Result.raw_declaration_t)) =
  (pp_type_expr, ppf, (name, decl)) =>
    switch (decl) {
    | Constant(expr) => (name, expr) |> KConstant.Plugin.pp(ppf)
    | Enumerated(variants) =>
      (name, variants) |> KEnumerated.Plugin.pp(pp_type_expr, ppf)
    | Function(args, expr) =>
      (name, args, expr) |> KFunction.Plugin.pp(KExpression.Plugin.pp, ppf)
    | View(props, mixins, expr) =>
      (name, props, mixins, expr)
      |> KView.Plugin.pp(KExpression.Plugin.pp, ppf)
    };
