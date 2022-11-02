open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;

let string: Parse.Kore.primitive_parser_t =
  Matchers.string >|= Node.map(AST.Raw.of_string);
