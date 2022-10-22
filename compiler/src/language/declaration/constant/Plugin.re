open Knot.Kore;

let parse = Parser.constant;

let pp: Fmt.t((string, AST.expression_t)) =
  (ppf, (name, (expr, _))) =>
    Fmt.pf(ppf, "const %s = %a;", name, KExpression.Plugin.pp, expr);
