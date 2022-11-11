open AST;

let pp = Formatter.pp_enumerated;

include Framework.Declaration({
  type value_t =
    list((Result.identifier_t, list(Result.node_t(TypeExpression.raw_t))));

  let parse = Parser.enumerated;

  let format = pp;

  let to_xml = Debug.to_xml;
});
