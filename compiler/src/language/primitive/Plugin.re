include Interface;

let parse_primitive = Parser.parse_primitive;
let analyze_primitive = Analyzer.analyze_primitive;
let format_primitive = Formatter.format_primitive;
let primitive_to_xml = Debug.primitive_to_xml;

include AST.Framework.Expression.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
