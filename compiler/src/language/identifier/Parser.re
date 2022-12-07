open Knot.Kore;
open Parse.Kore;
open AST;

let parse_identifier = (ctx: ParseContext.t) =>
  Matchers.identifier(
    ~prefix=
      Matchers.alpha
      <|> Matchers.underscore
      <|> char(Constants.Character.dollar_sign),
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

let parse = (ctx: ParseContext.t) =>
  parse_identifier(ctx) >|= Node.map(Raw.of_id);
