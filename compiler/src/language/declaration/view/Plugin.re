let analyze_view_body = Analyzer.analyze_view_body;

include Interface;
include AST.Framework.Declaration.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
