open Knot.Kore;
open AST;

let analyze = Analyzer.analyze_function_call;

let pp = Formatter.pp_function_call;

include Framework.Expression({
  type parse_arg_t = (
    Framework.expression_parser_t,
    Framework.expression_parser_t,
  );

  type value_t('a) = (
    Expression.expression_t('a),
    list(Expression.expression_t('a)),
  );

  let parse = Parser.function_call;

  let format = pp;

  let to_xml = Debug.to_xml;
});
