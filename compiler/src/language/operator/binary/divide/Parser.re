open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let divide = Util.binary_op(AST.Raw.of_div_op) <$ Symbol.divide;
