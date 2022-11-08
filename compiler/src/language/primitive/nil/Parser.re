open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;

let nil: primitive_parser_t =
  Matchers.keyword(Constants.Keyword.nil) >|= Node.map(_ => AST.Raw.nil);
