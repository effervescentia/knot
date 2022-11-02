open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Symbol = Parse.Symbol;

let arguments =
    (
      ctx: ParseContext.t,
      parse_expression: Parse.Kore.contextual_expression_parser_t,
    ) =>
  KIdentifier.Plugin.parse(ctx)
  >>= (
    id =>
      Symbol.colon
      >> KTypeExpression.Plugin.parse
      >|= ((type_, default) => (id, Some(type_), default))
      |> option(default => (id, None, default))
  )
  >>= (
    f =>
      Symbol.assign
      >> parse_expression(ctx)
      >|= Option.some
      >|= f
      |> option(f(None))
  )
  >|= (
    ((name, type_, default)) => {
      let name_range = Node.get_range(name);

      Node.typed(
        AST.Raw.{name, default, type_},
        (),
        Range.join(
          name_range,
          default
          |?> Node.get_range
          |?: (type_ |?> Node.get_range |?: name_range),
        ),
      );
    }
  )
  |> Matchers.comma_sep
  |> Matchers.between(Symbol.open_group, Symbol.close_group);

let _full_parser =
    (
      ~mixins,
      ctx: ParseContext.t,
      parse_expression: Parse.Kore.contextual_expression_parser_t,
    ) =>
  arguments(ctx, parse_expression)
  >|= fst
  |> option([])
  >>= (
    args =>
      (
        mixins
          ? Symbol.mixin >> Matchers.identifier |> many1 |> option([])
          : return([])
      )
      >>= (
        mixins =>
          Matchers.glyph("->")
          >>= (
            lambda =>
              parse_expression(ctx)
              >|= (
                expr => (args, mixins, expr, Node.join_ranges(lambda, expr))
              )
          )
      )
  );

let lambda_with_mixins = (ctx: ParseContext.t) =>
  _full_parser(~mixins=true, ctx);

let lambda =
    (
      ctx: ParseContext.t,
      parse_expression: Parse.Kore.contextual_expression_parser_t,
    ) =>
  _full_parser(~mixins=false, ctx, parse_expression)
  >|= (((args, _, expr, range)) => (args, expr, range));
