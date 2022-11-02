open Knot.Kore;
open Parse.Onyx;

module Keyword = Parse.Keyword;
module Matchers = Parse.Matchers;

let variable =
    (
      ctx: ParseContext.t,
      parse_expr: Parse.Kore.contextual_expression_parser_t,
    )
    : Parse.Kore.statement_parser_t =>
  Keyword.let_
  >>= (
    kwd =>
      Matchers.assign(KIdentifier.Plugin.parse(ctx), parse_expr(ctx))
      >|= (
        ((_, expr) as var) =>
          Node.typed(AST.Raw.of_var(var), (), Node.join_ranges(kwd, expr))
      )
  );
