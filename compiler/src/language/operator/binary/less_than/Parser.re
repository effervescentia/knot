open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let less_than = Util.binary_op(AST.Raw.of_lt_op) <$ Symbol.less_than;
