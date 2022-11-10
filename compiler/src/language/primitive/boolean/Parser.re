open Knot.Kore;
open Parse.Kore;
open AST;

module Keyword = Constants.Keyword;

let boolean_literal = (keyword, value) =>
  Matchers.keyword(keyword) >|= Node.map(_ => Raw.of_bool(value));

let boolean =
  boolean_literal(Keyword.true_, true)
  <|> boolean_literal(Keyword.false_, false);
