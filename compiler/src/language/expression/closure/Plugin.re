open AST;

let analyze = Analyzer.analyze;

include Framework.Expression({
  type parse_arg_t = (
    ParseContext.t,
    Framework.contextual_expression_parser_t,
  );

  type value_t('expr, 'typ) = list(Statement.node_t('expr, 'typ));

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
