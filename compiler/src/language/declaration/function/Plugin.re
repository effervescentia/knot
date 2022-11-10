open AST;

include Framework.Declaration({
  type value_t = (list(Result.argument_t), Result.expression_t);

  let parse = Parser.function_;

  let pp = Formatter.pp_function;

  let to_xml = Debug.to_xml;
});
