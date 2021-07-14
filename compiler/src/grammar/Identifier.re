open Kore;

let reserved = (ctx: Context.t) =>
  choice(Constants.Keyword.reserved |> List.map(M.keyword))
  >|= (
    name => {
      ctx.report(
        ParseError(
          ReservedKeyword(name |> Block.value),
          ctx.namespace,
          name |> Block.cursor,
        ),
      );

      name;
    }
  );

let named = M.identifier;

let parser = (ctx: Context.t) =>
  choice([reserved(ctx), named])
  >|= Tuple.split2(Block.value % Reference.Identifier.of_string, Block.cursor);
