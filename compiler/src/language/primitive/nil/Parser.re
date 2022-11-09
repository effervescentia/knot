open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

let nil: primitive_parser_t =
  Matchers.keyword(Constants.Keyword.nil) >|= Node.map(_ => AST.Raw.nil);
