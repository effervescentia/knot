open Knot.Kore;
open Parse.Kore;
open AST;

let closure =
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
  >|= (
    ((stmts, _) as stmts_node) =>
      Node.typed(Raw.of_closure(stmts), (), Node.get_range(stmts_node))
  );
