open AST;

include Framework.DeclarationV2({
  type value_t = (list(Result.argument_t), Result.expression_t);

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
