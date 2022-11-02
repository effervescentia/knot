open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let logical_not = Util.unary_op(AST.Raw.of_not_op) <$ Symbol.not;
