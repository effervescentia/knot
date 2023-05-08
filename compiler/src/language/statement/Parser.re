open Kore;
open Parse.Kore;
open AST;

let parse =
    (
      ctx: ParseContext.t('ast),
      parse_expr: Framework.Interface.contextual_parse_t('ast, 'expr),
    ) => {
  let (&>) = (parse, to_statement) =>
    (ctx, parse_expr) |> parse >|= Node.map(to_statement);

  choice([
    Variable.parse &> Interface.of_variable,
    Effect.parse &> Interface.of_effect,
  ])
  |> Matchers.terminated;
};
