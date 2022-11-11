open AST;

let pp = Formatter.pp_function;

include Framework.Declaration({
  type value_t = (list(Result.argument_t), Result.expression_t);

  let parse = Parser.function_;

  let format = pp;

  let to_xml = Debug.to_xml;
});
