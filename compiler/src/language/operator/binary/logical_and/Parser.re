open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Util = Grammar.Util;

let logical_and = (ctx: ParseContext.t) =>
  AST.Raw.of_and_op |> Util.binary_op(ctx) <$ Matchers.glyph("&&");
