open Kore;

let reserved = (ctx: ModuleContext.t) =>
  choice(Constants.Keyword.reserved |> List.map(M.keyword))
  >|= (
    name => {
      ParseError(
        ReservedKeyword(NR.get_value(name)),
        ctx.namespace_context.namespace,
        NR.get_range(name),
      )
      |> ModuleContext.report(ctx);

      name;
    }
  );

let named = M.identifier;

let parser = (ctx: ModuleContext.t): identifier_parser_t =>
  choice([reserved(ctx), named])
  >|= NR.map_value(Reference.Identifier.of_string);
