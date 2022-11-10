open AST;

include Framework.Declaration({
  type value_t = (
    list(Result.argument_t),
    list(Result.node_t(string)),
    Result.expression_t,
  );

  let parse = Parser.view;

  let pp = Formatter.pp_view;

  let to_xml = Debug.to_xml;
});
