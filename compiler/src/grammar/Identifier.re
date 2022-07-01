open Kore;

let parser = (ctx: ParseContext.t): identifier_parser_t =>
  M.identifier(
    ~prefix=M.alpha <|> Character.underscore <|> Character.dollar_sign,
  )
  >|= (
    ((name_value, _) as name) => {
      if (Constants.Keyword.reserved |> List.mem(name_value)) {
        ctx
        |> ParseContext.report(
             ReservedKeyword(name_value),
             N.get_range(name),
           );
      };

      N.untyped(name_value, N.get_range(name));
    }
  );
