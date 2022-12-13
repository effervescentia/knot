open Knot.Kore;
open Parse.Kore;
open AST;

let parse: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_gte_op) <$ Matchers.glyph(">=");
