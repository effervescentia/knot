open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;

let integer =
  many1(Matchers.digit)
  >|= Input.join
  >|= Node.map(Int64.of_string % AST.Raw.of_int)
  >|= Node.add_type(Type.Raw.(`Integer))
  |> Matchers.lexeme;