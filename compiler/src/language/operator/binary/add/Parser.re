open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let add = Util.binary_op(AST.Raw.of_add_op) <$ Symbol.add;
