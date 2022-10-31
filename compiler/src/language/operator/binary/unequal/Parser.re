open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Util = Grammar.Util;

let unequal = Util.binary_op(AST.Raw.of_ineq_op) <$ Matchers.glyph("!=");
