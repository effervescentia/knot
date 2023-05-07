let validate_ksx_primitive_expression = Validator.validate_ksx_primitive_expression;
let validate_ksx_render = Validator.validate_ksx_render;

let analyze_ksx = Analyzer.analyze_ksx;

include Interface;
include AST.Framework.Expression.Make({
  include Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
