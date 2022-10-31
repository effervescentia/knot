open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Util = Grammar.Util;

let logical_or = Util.binary_op(AST.Raw.of_or_op) <$ Matchers.glyph("||");
