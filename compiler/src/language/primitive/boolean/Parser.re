open Knot.Kore;
open Parse.Kore;

module Keyword = Constants.Keyword;

let parse: Interface.Plugin.parse_t =
  choice([
    true <$| Matchers.keyword(Keyword.true_),
    false <$| Matchers.keyword(Keyword.false_),
  ]);
