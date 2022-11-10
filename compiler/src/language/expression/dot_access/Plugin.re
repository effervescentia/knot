open Knot.Kore;
open AST;

let analyze = Analyzer.analyze_dot_access;

include Framework.Expression({
  type parse_arg_t = Raw.expression_t;
  type pp_arg_t = Fmt.t(Result.raw_expression_t);

  type value_t('a) = (
    Expression.expression_t('a),
    Common.untyped_t(string),
  );

  let parse = Parser.dot_access;

  let pp = Formatter.pp_dot_access;

  let to_xml = Debug.to_xml;
});
