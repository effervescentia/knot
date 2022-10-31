open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let divide = Util.binary_op(AST.Raw.of_div_op) <$ Symbol.divide;
