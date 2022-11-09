open Knot.Kore;
open Parse.Kore;

module Util = Parse.Util;

let expo:
  Parse.Parser.t(
    (AST.Raw.expression_t, AST.Raw.expression_t) => AST.Raw.expression_t,
  ) =
  Tuple.fold2(Util.binary_op(AST.Raw.of_expo_op))
  <$ Matchers.symbol(Constants.Character.caret);
