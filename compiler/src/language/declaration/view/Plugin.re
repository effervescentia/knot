include AST.Framework.Declaration.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
