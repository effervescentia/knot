open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let expo:
  Parse.Kore.parser_t(
    (AST.Raw.expression_t, AST.Raw.expression_t) => AST.Raw.expression_t,
  ) =
  Tuple.fold2(Util.binary_op(AST.Raw.of_expo_op))
  <$ Matchers.symbol(Constants.Character.caret);
