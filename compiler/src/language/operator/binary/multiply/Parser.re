open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let multiply = Util.binary_op(AST.Raw.of_mult_op) <$ Symbol.multiply;
