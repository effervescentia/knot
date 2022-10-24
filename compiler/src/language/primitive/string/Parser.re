open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;

let string: Grammar.Kore.primitive_parser_t =
  Matchers.string
  >|= Node.map(AST.Raw.of_string)
  >|= Node.add_type(Type.Raw.(`String));
