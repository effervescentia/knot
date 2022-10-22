open Knot.Kore;
open Parse.Onyx;

module Character = Grammar.Character;
module Matchers = Grammar.Matchers;

let parse = (ctx: ParseContext.t): Grammar.Kore.identifier_parser_t =>
  Matchers.identifier(
    ~prefix=Matchers.alpha <|> Character.underscore <|> Character.dollar_sign,
  )
  >|= (
    ((name_value, _) as name) => {
      if (Constants.Keyword.reserved |> List.mem(name_value)) {
        ctx
        |> ParseContext.report(
             ReservedKeyword(name_value),
             Node.get_range(name),
           );
      };

      Node.untyped(name_value, Node.get_range(name));
    }
  );

let pp: Fmt.t(string) = Fmt.string;
