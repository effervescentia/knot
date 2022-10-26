open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Util = Grammar.Util;

let logical_or = (ctx: ParseContext.t) =>
  AST.Raw.of_or_op |> Util.binary_op(ctx) <$ Matchers.glyph("||");
