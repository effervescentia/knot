open Knot.Kore;
open Parse.Onyx;

module ParseContext = AST.ParseContext;
module Character = Parse.Character;
module Matchers = Parse.Matchers;

let identifier = (ctx: ParseContext.t) =>
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
