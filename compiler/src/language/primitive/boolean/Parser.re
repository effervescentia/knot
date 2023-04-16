open Knot.Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t =
  choice([
    true <$| Matchers.keyword(Constants.Keyword.true_),
    false <$| Matchers.keyword(Constants.Keyword.false_),
  ]);
