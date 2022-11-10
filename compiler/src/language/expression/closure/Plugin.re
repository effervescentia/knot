open AST;

let parse = Parser.closure;

let analyze = Analyzer.analyze_closure;

let pp = Formatter.pp_closure;

let to_xml = Debug.to_xml;

include Framework.Expression({
  type parse_arg_t = (
    ParseContext.t,
    ParserTypes.contextual_expression_parser_t,
  );

  type value_t('a) = list(Expression.statement_t('a));

  let parse = Parser.closure;

  let format = Formatter.pp_closure;

  let to_xml = Debug.to_xml;
});
