open Knot.Kore;
open Parse.Kore;
open AST;

let unequal: ParserTypes.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_ineq_op) <$ Matchers.glyph("!=");
