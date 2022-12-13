open Kore;
open Parse.Kore;
open AST;

let parse =
    (
      ctx: ParseContext.t,
      parse_expr: Framework.contextual_expression_parser_t,
    )
    : Framework.statement_parser_t => {
  let arg = (ctx, parse_expr);

  choice([KVariable.parse(arg), KEffect.parse(arg)]) |> Matchers.terminated;
};
