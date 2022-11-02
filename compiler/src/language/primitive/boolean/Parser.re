open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Keyword = Parse.Keyword;

let boolean: primitive_parser_t =
  Keyword.true_
  >|= Node.map(_ => AST.Raw.of_bool(true))
  <|> (Keyword.false_ >|= Node.map(_ => AST.Raw.of_bool(false)));
