open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;
module Matchers = Grammar.Matchers;

let variable =
    (
      ctx: ParseContext.t,
      parse_expr: Grammar.Kore.contextual_expression_parser_t,
    )
    : Grammar.Kore.statement_parser_t =>
  Keyword.let_
  >>= (
    kwd =>
      Matchers.assign(KIdentifier.Plugin.parse(ctx), parse_expr(ctx))
      >|= (
        ((_, expr) as var) =>
          Node.typed(
            AST.Raw.of_var(var),
            Type.Raw.(`Nil),
            Node.join_ranges(kwd, expr),
          )
      )
  );
