open Knot.Kore;
open Parse.Kore;
open AST;

let statement =
    (
      ctx: ParseContext.t,
      parse_expr: ParserTypes.contextual_expression_parser_t,
    )
    : ParserTypes.statement_parser_t => {
  let arg = (ctx, parse_expr);

  choice([KVariable.Plugin.parse(arg), KEffect.Plugin.parse(arg)])
  |> Matchers.terminated;
};
