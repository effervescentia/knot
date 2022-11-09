open Knot.Kore;
open Parse.Kore;

let decorator = parse_expr =>
  Matchers.identifier(~prefix=char(Constants.Character.at_sign))
  >|= Node.map(String.drop_prefix("@"))
  >>= (
    id =>
      parse_expr
      |> Matchers.comma_sep
      |> Matchers.between_parentheses
      |> option(Node.untyped([], Node.get_range(id)))
      >|= (
        args => Node.untyped((id, fst(args)), Node.join_ranges(id, args))
      )
  );
