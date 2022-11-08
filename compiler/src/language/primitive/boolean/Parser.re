open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;

let boolean: primitive_parser_t =
  Matchers.keyword(Constants.Keyword.true_)
  >|= Node.map(_ => AST.Raw.of_bool(true))
  <|> (
    Matchers.keyword(Constants.Keyword.false_)
    >|= Node.map(_ => AST.Raw.of_bool(false))
  );
