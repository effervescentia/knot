let parse_logical = Parser.parse_logical;
let parse_comparison = Parser.parse_comparison;
let parse_relational = Parser.parse_relational;
let parse_arithmetic = Parser.parse_arithmetic;

include AST.Framework.NoParseExpression.Make({
  include Interface.Plugin;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
