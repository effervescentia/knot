open Kore;

let reserved = (ctx: ModuleContext.t) =>
  choice(Constants.Keyword.reserved |> List.map(M.keyword))
  >|= (
    ((name_value, name_range) as name) => {
      ParseError(
        ReservedKeyword(name_value),
        ctx.namespace_context.namespace,
        name_range,
      )
      |> ModuleContext.report(ctx);

      name;
    }
  );

let named = M.identifier;

let parser = (ctx: ModuleContext.t): identifier_parser_t =>
  choice([reserved(ctx), named])
  >|= NR.map_value(Reference.Identifier.of_string);
