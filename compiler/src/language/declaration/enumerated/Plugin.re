open AST;

include Framework.DeclarationV2({
  type value_t =
    list((Result.identifier_t, list(Result.node_t(TypeExpression.raw_t))));

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
