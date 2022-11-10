open Knot.Kore;
open AST;

let analyze = Analyzer.analyze_binary_operation;

include Framework.NoParseExpression({
  type pp_arg_t = Fmt.t(Result.raw_expression_t);

  type value_t('a) = (
    Operator.Binary.t,
    Expression.expression_t('a),
    Expression.expression_t('a),
  );

  let format = Formatter.pp_binary_operation;

  let to_xml = Debug.to_xml;
});
