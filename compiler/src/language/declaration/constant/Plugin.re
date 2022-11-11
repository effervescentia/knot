open AST;

let pp = Formatter.pp_constant;

include Framework.Declaration({
  type value_t = Result.expression_t;

  let parse = Parser.constant;

  let format = pp;

  let to_xml = Debug.to_xml;
});
