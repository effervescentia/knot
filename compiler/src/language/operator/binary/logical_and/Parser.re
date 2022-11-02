open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let logical_and = Util.binary_op(AST.Raw.of_and_op) <$ Matchers.glyph("&&");
