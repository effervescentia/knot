open Kore;

let reserved = (ctx: ModuleContext.t) =>
  choice(Constants.Keyword.reserved |> List.map(M.keyword))
  >|= (
    name => {
      ParseError(
        ReservedKeyword(Node.Raw.value(name)),
        ctx.namespace_context.namespace,
        Node.Raw.cursor(name),
      )
      |> ModuleContext.report(ctx);

      name;
    }
  );

let named = M.identifier;

let parser = (ctx: ModuleContext.t) =>
  choice([reserved(ctx), named])
  >|= Tuple.split2(
        Node.Raw.value % Reference.Identifier.of_string,
        Node.Raw.cursor,
      );
