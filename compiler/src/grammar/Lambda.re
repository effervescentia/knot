open Kore;
open AST.Raw;

let arguments = (ctx: ClosureContext.t) =>
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
          let name_cursor = Block.cursor(name);

          (
            {
              /* let type_ =
                 switch (default) {
                 | Some((_, t, _)) =>
                   ctx |> ClosureContext.define(id, t);
                   t;
                 | None => ctx |> ClosureContext.define_weak(id)
                 }; */

              name,
              default,
              type_: None,
            },
            Cursor.join(
              name_cursor,
              default |> Option.map(Block.cursor) |?: name_cursor,
            ),
          );
        }
      ),
      Identifier.parser(ctx)
      >|= (
        name => (
          {
            name,
            default: None,
            type_: None,
            /* ctx |> ClosureContext.define_weak(id), */
          },
          Block.cursor(name),
        )
      ),
    ])
    |> sep_by(Symbol.comma),
  )
  >|= Block.value;

let parser = (ctx: ClosureContext.t) =>
  option([], arguments(ctx))
  >>= (
    args => Glyph.lambda >> Expression.parser(ctx) >|= (expr => (args, expr))
  );
