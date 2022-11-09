open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

let boolean: primitive_parser_t =
  Matchers.keyword(Constants.Keyword.true_)
  >|= Node.map(_ => AST.Raw.of_bool(true))
  <|> (
    Matchers.keyword(Constants.Keyword.false_)
    >|= Node.map(_ => AST.Raw.of_bool(false))
  );
