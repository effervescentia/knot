open AST;

let pp = Formatter.format;

include Framework.Declaration({
  type value_t = (
    list(Result.argument_t),
    list(Result.node_t(string)),
    Result.expression_t,
  );

  let parse = Parser.parse;

  let format = pp;

  let to_xml = Debug.to_xml;
});
