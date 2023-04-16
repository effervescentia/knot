module ViewKind = Interface.ViewKind;
module Attribute = Interface.Attribute;
module Child = Interface.Child;

include AST.Framework.Expression.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
