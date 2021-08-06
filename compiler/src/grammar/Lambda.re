open Kore;
open AST.Raw;

let arguments = (ctx: ModuleContext.t) =>
  M.between(
    Symbol.open_group,
    Symbol.close_group,
    choice([
      Operator.assign(
        Identifier.parser(ctx),
        Expression.parser(ctx) >|= Option.some,
      )
      >|= (
        ((name, default)) => {
          let name_range = Node.Raw.range(name);

          (
            {name, default, type_: None},
            Range.join(
              name_range,
              default |> Option.map(Node.Raw.range) |?: name_range,
            ),
          );
        }
      ),
      Identifier.parser(ctx)
      >|= (
        name => ({name, default: None, type_: None}, Node.Raw.range(name))
      ),
    ])
    |> sep_by(Symbol.comma),
  )
  >|= Node.Raw.value;

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
              Range.join(Node.Raw.range(start), Node.Raw.range(expr)),
            )
          )
      )
  );
