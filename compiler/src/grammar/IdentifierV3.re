open Kore;

module Identifier = Reference.Identifier;

let reserved = (ctx: ModuleContext.t): string_parser_t =>
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

let named: string_parser_t = M.identifier;

let untyped_parser = (ctx: ModuleContext.t): untyped_identifier_parser_t =>
  choice([reserved(ctx), named]) >|= NR.map_value(Identifier.of_string);

let parser = (ctx: ModuleContext.t): identifier_parser_t =>
  untyped_parser(ctx) >|= N.of_raw(TR.Valid(`Unknown));
