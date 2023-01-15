open Knot.Kore;

include AST.Framework.Expression.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = ((expr_to_xml, _), expression) =>
    Fmt.Node("Group", [], [expr_to_xml(expression)]);
});
