open Knot.Kore;
open Parse.Kore;
open AST;

let boolean: ParserTypes.primitive_parser_t =
  Matchers.keyword(Constants.Keyword.true_)
  >|= Node.map(_ => Raw.of_bool(true))
  <|> (
    Matchers.keyword(Constants.Keyword.false_)
    >|= Node.map(_ => Raw.of_bool(false))
  );
