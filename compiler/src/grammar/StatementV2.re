open Kore;

let variable =
    (ctx: ModuleContext.t, parse_expr: contextual_expression_parser_t)
    : statement_parser_t =>
  Keyword.let_
  >|= NR.get_range
  >>= (
    start_range =>
      OperatorV2.assign(IdentifierV2.parser(ctx), parse_expr(ctx))
      >|= (
        ((_, expr) as var) =>
          N.create(
            AR.of_var(var),
            TR.(`Nil),
            Range.join(start_range, N.get_range(expr)),
          )
      )
  );

let expression =
    (ctx: ModuleContext.t, parse_expr: contextual_expression_parser_t)
    : statement_parser_t =>
  parse_expr(ctx) >|= N.wrap(AR.of_expr);

let parser =
    (ctx: ModuleContext.t, parse_expr: contextual_expression_parser_t)
    : statement_parser_t =>
  choice([variable(ctx, parse_expr), expression(ctx, parse_expr)])
  |> M.terminated;
