open Knot.Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  ((ctx, parse_expr)) => parse_expr(ctx) >|= Node.wrap(Fun.id);
