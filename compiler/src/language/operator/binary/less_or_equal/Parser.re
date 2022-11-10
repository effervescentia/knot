open Knot.Kore;
open Parse.Kore;
open AST;

let less_or_equal: ParserTypes.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_lte_op) <$ Matchers.glyph("<=");
