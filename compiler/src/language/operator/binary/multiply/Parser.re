open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let multiply = Util.binary_op(AST.Raw.of_mult_op) <$ Symbol.multiply;
