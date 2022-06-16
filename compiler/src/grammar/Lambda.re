open Kore;

let arguments = (ctx: ModuleContext.t) =>
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
      let name_range = N2.get_range(name);

      N2.typed(
        AR.{name, default, type_},
        default |?> N2.get_type |?: TR.(`Unknown),
        Range.join(
          name_range,
          default |?> N2.get_range |?: (type_ |?> N2.get_range |?: name_range),
        ),
      );
    }
  )
  |> M.comma_sep
  |> M.between(Symbol.open_group, Symbol.close_group)
  >|= fst;

let parser = (ctx: ModuleContext.t) =>
  option([], arguments(ctx))
  >>= (
    args =>
      Glyph.lambda
      >>= (
        lambda =>
          Expression.parser(ctx)
          >|= (expr => (args, expr, N2.join_ranges(lambda, expr)))
      )
  );
