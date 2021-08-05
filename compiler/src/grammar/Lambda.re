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
          let name_cursor = Node.Raw.cursor(name);

          (
            {name, default, type_: None},
            Cursor.join(
              name_cursor,
              default |> Option.map(Node.Raw.cursor) |?: name_cursor,
            ),
          );
        }
      ),
      Identifier.parser(ctx)
      >|= (
        name => ({name, default: None, type_: None}, Node.Raw.cursor(name))
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
              Cursor.join(Node.Raw.cursor(start), Node.Raw.cursor(expr)),
            )
          )
      )
  );
