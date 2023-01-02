open Kore;
open AST;

let format: Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_statement_t) =
  (pp_expression, ppf, stmt) =>
    switch (stmt) {
    | Variable(name, expr) =>
      (name, expr) |> KVariable.format(pp_expression, ppf)
    | Effect(expr) => expr |> KEffect.format(pp_expression, ppf)
    };
