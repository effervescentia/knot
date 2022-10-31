open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let logical_not = Util.unary_op(AST.Raw.of_not_op) <$ Symbol.not;
