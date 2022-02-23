open Kore;
open AST.Raw;

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
      let name_range = Node.Raw.get_range(name);

      (
        {name, default, type_: None},
        Range.join(
          name_range,
          default
          |> Option.map(Node.Raw.get_range)
          |?: (type_ |> Option.map(Node.Raw.get_range) |?: name_range),
        ),
      );
    }
  )
  |> sep_by(Symbol.comma)
  |> M.between(Symbol.open_group, Symbol.close_group)
  >|= Node.Raw.get_value;

let parser = (ctx: ModuleContext.t) =>
  option([], arguments(ctx))
  >>= (
    args =>
      Glyph.lambda
      >>= (
        start =>
          Expression.parser(ctx)
          >|= (
            expr => (
              args,
              expr,
              Node.Raw.(Range.join(get_range(start), get_range(expr))),
            )
          )
      )
  );
