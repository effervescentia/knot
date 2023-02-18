open Knot.Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  parse_expr =>
    Matchers.identifier(~prefix=char(Constants.Character.at_sign))
    >|= Node.map(String.drop_prefix("@"))
    >>= (
      id =>
        parse_expr
        |> Matchers.comma_sep
        |> Matchers.between_parentheses
        |> option(Node.raw([], Node.get_range(id)))
        >|= (args => Node.raw((id, fst(args)), Node.join_ranges(id, args)))
    );
