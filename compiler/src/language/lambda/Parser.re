open Knot.Kore;
open Parse.Kore;
open AST;

module Character = Constants.Character;

let parse_arguments =
    (
      ctx: ParseContext.t,
      parse_expression: Framework.contextual_expression_parser_t,
    ) =>
  KIdentifier.Parser.parse_raw(ctx)
  >>= (
    id =>
      Matchers.symbol(Character.colon)
      >> KTypeExpression.Plugin.parse
      >|= ((type_, default) => (id, Some(type_), default))
      |> option(default => (id, None, default))
  )
  >>= (
    f =>
      Matchers.symbol(Character.equal_sign)
      >> parse_expression(ctx)
      >|= Option.some
      >|= f
      |> option(f(None))
  )
  >|= (
    ((name, type_, default)) => {
      let name_range = Node.get_range(name);

      Node.typed(
        Expression.{name, default, type_},
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
  |> Matchers.between_parentheses;

let _parse_configurable_lambda =
    (
      ~mixins,
      ctx: ParseContext.t,
      parse_expression: Framework.contextual_expression_parser_t,
    ) =>
  parse_arguments(ctx, parse_expression)
  >|= fst
  |> option([])
  >>= (
    args =>
      (
        mixins
          ? Matchers.symbol(Character.tilde)
            >> Matchers.identifier
            |> many1
            |> option([])
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

let parse_lambda_with_mixins = (ctx: ParseContext.t) =>
  _parse_configurable_lambda(~mixins=true, ctx);

let parse_lambda =
    (
      ctx: ParseContext.t,
      parse_expression: Framework.contextual_expression_parser_t,
    ) =>
  _parse_configurable_lambda(~mixins=false, ctx, parse_expression)
  >|= (((args, _, expr, range)) => (args, expr, range));
