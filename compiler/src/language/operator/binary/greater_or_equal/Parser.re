open Knot.Kore;
open Parse.Kore;
open AST;

let greater_or_equal: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_gte_op) <$ Matchers.glyph(">=");
