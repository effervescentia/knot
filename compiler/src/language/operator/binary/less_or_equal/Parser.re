open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let less_or_equal =
  Util.binary_op(AST.Raw.of_lte_op) <$ Matchers.glyph("<=");
