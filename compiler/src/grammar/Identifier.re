open Kore;

let reserved = (ctx: ClosureContext.t) =>
  choice(Constants.Keyword.reserved |> List.map(M.keyword))
  >|= (
    name => {
      ParseError(
        ReservedKeyword(Block.value(name)),
        ctx.namespace_context.namespace,
        Block.cursor(name),
      )
      |> ClosureContext.report(ctx);

      name;
    }
  );

let named = M.identifier;

let parser = (ctx: ClosureContext.t) =>
  choice([reserved(ctx), named])
  >|= Tuple.split2(Block.value % Reference.Identifier.of_string, Block.cursor);
