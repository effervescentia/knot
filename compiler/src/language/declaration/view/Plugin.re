open AST;

include Framework.Declaration({
  type value_t = (
    list(Result.argument_t),
    list(Result.node_t(string)),
    Result.expression_t,
  );

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
