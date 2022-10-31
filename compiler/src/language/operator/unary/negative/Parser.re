open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let negative = Util.unary_op(AST.Raw.of_neg_op) <$ Symbol.negative;
