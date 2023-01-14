open Knot.Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  ((ctx, parse_expr)) =>
    Matchers.keyword(Constants.Keyword.let_)
    >>= (
      kwd =>
        Matchers.assign(KIdentifier.Parser.parse_raw(ctx), parse_expr(ctx))
        >|= (
          ((_, expression) as node) =>
            Node.raw(node, Node.join_ranges(kwd, expression))
        )
    );
