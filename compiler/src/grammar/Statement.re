open Kore;

let variable =
    (ctx: ModuleContext.t, parse_expr: contextual_expression_parser_t)
    : statement_parser_t =>
  Keyword.let_
  >>= (
    kwd =>
      Operator.assign(Identifier.parser(ctx), parse_expr(ctx))
      >|= (
        ((_, expr) as var) =>
          N2.typed(AR.of_var(var), TR.(`Nil), N2.join_ranges(kwd, expr))
      )
  );

let expression =
    (ctx: ModuleContext.t, parse_expr: contextual_expression_parser_t)
    : statement_parser_t =>
  parse_expr(ctx) >|= N2.wrap(AR.of_expr);

let parser =
    (ctx: ModuleContext.t, parse_expr: contextual_expression_parser_t)
    : statement_parser_t =>
  choice([variable(ctx, parse_expr), expression(ctx, parse_expr)])
  |> M.terminated;
