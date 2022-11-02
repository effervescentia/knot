open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let expo =
  Tuple.fold2(Util.binary_op(AST.Raw.of_expo_op)) <$ Symbol.exponent;
