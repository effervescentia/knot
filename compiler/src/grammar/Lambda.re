open Kore;

let arguments = (ctx: ParseContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      Symbol.colon
      >> Typing.expression_parser
      >|= ((type_, default) => (id, Some(type_), default))
      |> option(default => (id, None, default))
  )
  >>= (
    f =>
      Symbol.assign
      >> Expression.parser(ctx)
      >|= Option.some
      >|= f
      |> option(f(None))
  )
  >|= (
    ((name, type_, default)) => {
      let name_range = N.get_range(name);

      N.typed(
        AR.{name, default, type_},
        default |?> N.get_type |?: TR.(`Unknown),
        Range.join(
          name_range,
          default |?> N.get_range |?: (type_ |?> N.get_range |?: name_range),
        ),
      );
    }
  )
  |> M.comma_sep
  |> M.between(Symbol.open_group, Symbol.close_group);

let _full_parser = (~mixins, ctx: ParseContext.t) =>
  arguments(ctx)
  >|= fst
  |> option([])
  >>= (
    args =>
      (
        mixins
          ? Symbol.mixin >> M.identifier |> many1 |> option([]) : return([])
      )
      >>= (
        mixins =>
          Glyph.lambda
          >>= (
            lambda =>
              Expression.parser(ctx)
              >|= (expr => (args, mixins, expr, N.join_ranges(lambda, expr)))
          )
      )
  );

let parser_with_mixins = (ctx: ParseContext.t) =>
  _full_parser(~mixins=true, ctx);

let parser = (ctx: ParseContext.t) =>
  _full_parser(~mixins=false, ctx)
  >|= (((args, _, expr, range)) => (args, expr, range));
