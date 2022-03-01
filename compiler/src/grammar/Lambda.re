open Kore;

let arguments = (ctx: ModuleContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      Typing.expression_parser
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
      let name_range = NR.get_range(name);

      N.create(
        AR.{name, default, type_: None},
        default |> Option.map(N.get_type) |?: TR.(`Unknown),
        Range.join(
          name_range,
          default
          |> Option.map(N.get_range)
          |?: (type_ |> Option.map(NR.get_range) |?: name_range),
        ),
      );
    }
  )
  |> sep_by(Symbol.comma)
  |> M.between(Symbol.open_group, Symbol.close_group)
  >|= NR.get_value;

let parser = (ctx: ModuleContext.t) =>
  option([], arguments(ctx))
  >>= (
    args =>
      Glyph.lambda
      >|= NR.get_range
      >>= (
        start_range =>
          Expression.parser(ctx)
          >|= (
            expr => (args, expr, Range.join(start_range, N.get_range(expr)))
          )
      )
  );
