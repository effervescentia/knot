open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;

let boolean: Grammar.Kore.primitive_parser_t =
  Keyword.true_
  >|= Node.map(_ => AST.Raw.of_bool(true))
  <|> (Keyword.false_ >|= Node.map(_ => AST.Raw.of_bool(false)));
