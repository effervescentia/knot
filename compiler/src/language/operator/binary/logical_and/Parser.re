open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Util = Grammar.Util;

let logical_and = Util.binary_op(AST.Raw.of_and_op) <$ Matchers.glyph("&&");
