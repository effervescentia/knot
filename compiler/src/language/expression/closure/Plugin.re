open AST;

let analyze = Analyzer.analyze_closure;

let pp = Formatter.pp_closure;

include Framework.Expression({
  type parse_arg_t = (
    ParseContext.t,
    Framework.contextual_expression_parser_t,
  );

  type value_t('a) = list(Expression.statement_t('a));

  let parse = Parser.closure;

  let format = pp;

  let to_xml = Debug.to_xml;
});
