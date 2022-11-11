open Knot.Kore;
open Parse.Kore;
open AST;

let equal: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_eq_op) <$ Matchers.glyph("==");
