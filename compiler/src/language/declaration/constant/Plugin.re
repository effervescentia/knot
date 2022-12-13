open AST;

include Framework.Declaration({
  type value_t = Result.expression_t;

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
