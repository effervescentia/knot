open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let unequal = Util.binary_op(AST.Raw.of_ineq_op) <$ Matchers.glyph("!=");
