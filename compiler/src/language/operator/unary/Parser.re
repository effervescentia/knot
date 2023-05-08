open Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  (f, parse_expr) =>
    choice([KLogicalNot.parse(f), KAbsolute.parse(f), KNegative.parse(f)])
    |> Matchers.unary_op(parse_expr);
