open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let add = Util.binary_op(AST.Raw.of_add_op) <$ Symbol.add;
