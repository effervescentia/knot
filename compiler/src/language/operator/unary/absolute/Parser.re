open Knot.Kore;
open Parse.Onyx;

module Symbol = Parse.Symbol;
module Util = Parse.Util;

let absolute = AST.Raw.of_pos_op |> Util.unary_op <$ Symbol.positive;
