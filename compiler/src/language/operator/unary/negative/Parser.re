open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let negative = Util.unary_op(AST.Raw.of_neg_op) <$ Symbol.negative;
