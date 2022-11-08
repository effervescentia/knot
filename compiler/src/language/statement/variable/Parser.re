open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module ParseContext = AST.ParseContext;
module Matchers = Parse.Matchers;

let variable =
    (ctx: ParseContext.t, parse_expr: contextual_expression_parser_t)
    : statement_parser_t =>
  Matchers.keyword(Constants.Keyword.let_)
  >>= (
    kwd =>
      Matchers.assign(KIdentifier.Plugin.parse(ctx), parse_expr(ctx))
      >|= (
        ((_, expr) as var) =>
          Node.typed(AST.Raw.of_var(var), (), Node.join_ranges(kwd, expr))
      )
  );
