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
        (((id, _) as name, default)) => {
          let type_ =
            switch (default) {
            | Some((_, t, _)) =>
              ctx |> ClosureContext.define(id, t);
              t;
            | None => ctx |> ClosureContext.define_weak(id)
            };

          ({name, default}, type_);
        }
      ),
      Identifier.parser(ctx)
      >|= (
        ((id, _) as name) => (
          {name, default: None},
          ctx |> ClosureContext.define_weak(id),
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
