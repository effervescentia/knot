open Kore;

let parser = (ctx: ModuleContext.t): identifier_parser_t =>
  M.identifier
  >|= (
    ((name_value, _) as name) => {
      if (Constants.Keyword.reserved |> List.mem(name_value)) {
        ParseError(
          ReservedKeyword(name_value),
          ctx.namespace_context.namespace,
          N2.get_range(name),
        )
        |> ModuleContext.report(ctx);
      };

      N2.untyped(name_value, N2.get_range(name))
      |> N2.map(Reference.Identifier.of_string);
    }
  );
