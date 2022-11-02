open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Keyword = Parse.Keyword;

let nil: primitive_parser_t = Keyword.nil >|= Node.map(_ => AST.Raw.nil);
