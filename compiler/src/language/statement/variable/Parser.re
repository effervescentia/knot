open Knot.Kore;
open Parse.Kore;
open AST;

let variable =
    (
      (
        ctx: ParseContext.t,
        parse_expr: ParserTypes.contextual_expression_parser_t,
      ),
    )
    : ParserTypes.statement_parser_t =>
  Matchers.keyword(Constants.Keyword.let_)
  >>= (
    kwd =>
      Matchers.assign(KIdentifier.Plugin.parse_id(ctx), parse_expr(ctx))
      >|= (
        ((_, expr) as var) =>
          Node.typed(Raw.of_var(var), (), Node.join_ranges(kwd, expr))
      )
  );
