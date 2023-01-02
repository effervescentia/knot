open Knot.Kore;
open Parse.Kore;
open AST;

let parse =
    (
      (
        ctx: ParseContext.t,
        parse_expr: Framework.contextual_expression_parser_t,
      ),
    )
    : Framework.expression_parser_t =>
  KStatement.Plugin.parse(ctx, parse_expr)
  |> many
  |> Matchers.between_braces
  >|= Node.map(Raw.of_closure);
