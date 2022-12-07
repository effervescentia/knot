open Knot.Kore;
open AST;

let format: Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_statement_t) =
  (pp_expression, ppf, stmt) =>
    switch (stmt) {
    | Variable(name, expr) =>
      (name, expr) |> KVariable.Plugin.format(pp_expression, ppf)
    | Expression(expr) => expr |> KEffect.Plugin.format(pp_expression, ppf)
    };
