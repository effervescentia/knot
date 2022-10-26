open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Util = Grammar.Util;

let unequal = (ctx: ParseContext.t) =>
  AST.Raw.of_ineq_op |> Util.binary_op(ctx) <$ Matchers.glyph("!=");
