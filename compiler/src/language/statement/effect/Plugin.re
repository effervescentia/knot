include Interface;
include AST.Framework.Statement.Make({
  include Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
