open Knot.Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t('ast, 'result) =
  (f, (ctx, parse_expr)) =>
    KStatement.Plugin.parse(ctx, parse_expr)
    |> many
    |> Matchers.between_braces
    >|= Node.map(f);
