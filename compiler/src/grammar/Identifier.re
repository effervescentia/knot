open Kore;

let parser = (ctx: ModuleContext.t): identifier_parser_t =>
  M.identifier
  >|= (
    ((name_value, name_range) as name) => {
      if (Constants.Keyword.reserved |> List.mem(name_value)) {
        ParseError(
          ReservedKeyword(name_value),
          ctx.namespace_context.namespace,
          name_range,
        )
        |> ModuleContext.report(ctx);
      };

      name |> NR.map_value(Reference.Identifier.of_string);
    }
  );
