open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let less_than = Util.binary_op(AST.Raw.of_lt_op) <$ Symbol.less_than;
