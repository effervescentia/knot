open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let subtract = Util.binary_op(AST.Raw.of_sub_op) <$ Symbol.subtract;
