open Knot.Kore;
open Parse.Kore;
open AST;

let bind_style_expression: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_add_op)
  <$ Matchers.glyph(Constants.Glyph.style_binding);

// let bind_style_literal: Framework.binary_op_parser_t =
//   Parse.Util.binary_op(Raw.of_add_op)
//   <$ Matchers.glyph(Constants.Glyph.style_binding);

// let bind_style = bind_style_literal <|> bind_style_expression;
let bind_style = bind_style_expression;
