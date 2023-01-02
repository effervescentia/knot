open Kore;
open Parse.Kore;
open AST;

let parse =
    (
      ctx: ParseContext.t,
      parse_expr: Framework.contextual_expression_parser_t,
    )
    : Framework.statement_parser_t => {
  let (&>) = (parse, to_statement) =>
    (ctx, parse_expr) |> parse >|= Node.map(to_statement);

  choice([
    KVariable.parse &> AST.Raw.of_var,
    KEffect.parse &> AST.Raw.of_effect,
  ])
  |> Matchers.terminated;
};
