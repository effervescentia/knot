open Knot.Kore;
open Parse.Onyx;

module Symbol = Grammar.Symbol;
module Util = Grammar.Util;

let expo = (ctx: ParseContext.t) =>
  Tuple.fold2(Util.binary_op(ctx, AST.Raw.of_expo_op)) <$ Symbol.exponent;
