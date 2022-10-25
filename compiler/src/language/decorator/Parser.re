open Knot.Kore;
open Parse.Onyx;

module Character = Grammar.Character;
module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

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
        args =>
          Node.untyped(
            (id |> Node.add_type(Type.Raw.(`Unknown)), fst(args)),
            Node.join_ranges(id, args),
          )
      )
  );
