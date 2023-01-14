open Kore;
open Parse.Kore;
open AST;

let parse =
    (
      ctx: ParseContext.t('ast),
      parse_expr: Framework.contextual_expression_parser_t('ast, 'expr, unit),
    ) => {
  let (&>) = (parse, to_statement) =>
    (ctx, parse_expr) |> parse >|= Node.map(to_statement);

  choice([
    KVariable.parse &> Interface.of_variable,
    KEffect.parse &> Interface.of_effect,
  ])
  |> Matchers.terminated;
};
