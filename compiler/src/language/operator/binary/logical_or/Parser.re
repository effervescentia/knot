open Knot.Kore;
open Parse.Kore;
open AST;

let logical_or: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_or_op) <$ Matchers.glyph("||");
