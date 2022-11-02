open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;

let integer =
  many1(Matchers.digit)
  >|= Input.join
  >|= Node.map(Int64.of_string % AST.Raw.of_int)
  |> Matchers.lexeme;
