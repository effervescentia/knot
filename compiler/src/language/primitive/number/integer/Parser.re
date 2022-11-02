open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;

let integer: number_parser_t =
  many1(Matchers.digit)
  >|= Input.join
  >|= Node.map(Int64.of_string % AST.Raw.of_int)
  |> Matchers.lexeme;
