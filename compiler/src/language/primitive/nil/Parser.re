open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;

let nil: Grammar.Kore.primitive_parser_t =
  Keyword.nil >|= Node.map(_ => AST.Raw.nil);
