open Knot.Kore;
open Parse.Onyx;

module Character = Parse.Character;
module Matchers = Parse.Matchers;
module Symbol = Parse.Symbol;

let decorator = parse_expr =>
  Matchers.identifier(~prefix=Character.at_sign)
  >|= Node.map(String.drop_prefix("@"))
  >>= (
    id =>
      parse_expr
      |> Matchers.comma_sep
      |> Matchers.between(Symbol.open_group, Symbol.close_group)
      |> option(Node.untyped([], Node.get_range(id)))
      >|= (
        args => Node.untyped((id, fst(args)), Node.join_ranges(id, args))
      )
  );
