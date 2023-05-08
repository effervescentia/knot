open Knot.Kore;
open Parse.Kore;
open AST;

let parse_raw = (ctx: ParseContext.t('ast)) =>
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

      name |> Node.map(_ => name_value);
    }
  );

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  (f, ctx) => parse_raw(ctx) >|= Node.map(f);
