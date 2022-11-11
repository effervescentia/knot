open Knot.Kore;
open Parse.Kore;
open AST;

let bind_style_expression: Framework.binary_op_parser_t =
  Parse.Util.binary_op(Raw.of_bind_style)
  <$ Matchers.glyph(Constants.Glyph.style_binding);

// let bind_style_literal: Framework.binary_op_parser_t =
//   Parse.Util.binary_op(Raw.of_add_op)
//   <$ Matchers.glyph(Constants.Glyph.style_binding);

let bind_style = next => chainl1(next, bind_style_expression);
