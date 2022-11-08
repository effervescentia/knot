open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Util = Parse.Util;

let greater_than: AST.ParserTypes.binary_op_parser_t =
  Util.binary_op(AST.Raw.of_gt_op)
  <$ Matchers.symbol(Constants.Character.close_chevron);
