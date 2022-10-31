open Knot.Kore;

let pp_statement: Fmt.t(AST.raw_expression_t) => Fmt.t(AST.raw_statement_t) =
  (pp_expression, ppf, stmt) =>
    switch (stmt) {
    | Variable(name, expr) =>
      (name, expr) |> KVariable.Plugin.pp(pp_expression, ppf)
    | Expression(expr) => expr |> KEffect.Plugin.pp(pp_expression, ppf)
    };
