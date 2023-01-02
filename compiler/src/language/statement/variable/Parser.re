open Knot.Kore;
open Parse.Kore;
open AST;

let parse =
    (
      (
        ctx: ParseContext.t,
        parse_expr: Framework.contextual_expression_parser_t,
      ),
    ) =>
  Matchers.keyword(Constants.Keyword.let_)
  >>= (
    kwd =>
      Matchers.assign(KIdentifier.Parser.parse_raw(ctx), parse_expr(ctx))
      >|= (
        ((_, expr) as var) => Node.raw(var, Node.join_ranges(kwd, expr))
      )
  );
