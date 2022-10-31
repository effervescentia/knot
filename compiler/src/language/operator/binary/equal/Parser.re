open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Util = Grammar.Util;

let equal = Util.binary_op(AST.Raw.of_eq_op) <$ Matchers.glyph("==");
