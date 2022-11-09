open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

module Character = Constants.Character;
module ParseContext = AST.ParseContext;

let arguments =
    (ctx: ParseContext.t, parse_expression: contextual_expression_parser_t) =>
  KIdentifier.Plugin.parse(ctx)
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
        AST.Expression.{name, default, type_},
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

let _full_parser =
    (
      ~mixins,
      ctx: ParseContext.t,
      parse_expression: contextual_expression_parser_t,
    ) =>
  arguments(ctx, parse_expression)
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

let lambda_with_mixins = (ctx: ParseContext.t) =>
  _full_parser(~mixins=true, ctx);

let lambda =
    (ctx: ParseContext.t, parse_expression: contextual_expression_parser_t) =>
  _full_parser(~mixins=false, ctx, parse_expression)
  >|= (((args, _, expr, range)) => (args, expr, range));
