open Knot.Kore;
open Parse.Kore;
open AST;

let nil: ParserTypes.primitive_parser_t =
  Matchers.keyword(Constants.Keyword.nil) >|= Node.map(_ => Raw.nil);
