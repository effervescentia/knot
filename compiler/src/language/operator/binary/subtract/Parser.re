open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let subtract = Util.binary_op(AST.Raw.of_sub_op) <$ Symbol.subtract;
