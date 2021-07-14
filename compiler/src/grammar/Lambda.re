open Kore;

let arguments = (ctx: Context.t) =>
  M.between(
    Symbol.open_group,
    Symbol.close_group,
    choice([
      Operator.assign(
        Identifier.parser(ctx),
        Expression.parser(ctx) >|= Option.some,
      )
      >|= (
        ((id, default)) => {
          let type_ =
            switch (default) {
            | Some((_, t, _)) =>
              ctx.scope |> Scope.define(id |> fst, t);
              t;
            | None => ctx.scope |> Scope.weak
            };

          (AST.{name: id, default}, type_);
        }
      ),
      Identifier.parser(ctx)
      >|= (id => (AST.{name: id, default: None}, ctx.scope |> Scope.weak)),
    ])
    |> sep_by(Symbol.comma),
  )
  >|= Block.value;

let parser = (ctx: Context.t) =>
  option([], arguments(ctx))
  >>= (
    args => Glyph.lambda >> Expression.parser(ctx) >|= (expr => (args, expr))
  );
