open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let greater_or_equal =
  Util.binary_op(AST.Raw.of_gte_op) <$ Matchers.glyph(">=");
