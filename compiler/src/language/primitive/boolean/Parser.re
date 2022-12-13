open Knot.Kore;
open Parse.Kore;
open AST;

module Keyword = Constants.Keyword;

let parse =
  choice([
    Raw.of_bool(true) <$| Matchers.keyword(Keyword.true_),
    Raw.of_bool(false) <$| Matchers.keyword(Keyword.false_),
  ]);
