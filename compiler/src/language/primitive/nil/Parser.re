open Knot.Kore;
open Parse.Onyx;

module Keyword = Parse.Keyword;

let nil: Parse.Kore.primitive_parser_t =
  Keyword.nil >|= Node.map(_ => AST.Raw.nil);
