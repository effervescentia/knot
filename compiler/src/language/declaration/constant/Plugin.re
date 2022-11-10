open AST;

include Framework.Declaration({
  type value_t = Result.expression_t;

  let parse = Parser.constant;

  let pp = Formatter.pp_constant;

  let to_xml = Debug.to_xml;
});
