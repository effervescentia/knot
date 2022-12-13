open Knot.Kore;
open AST;

let parse_logical = Parser.parse_logical;
let parse_comparison = Parser.parse_comparison;
let parse_relational = Parser.parse_relational;
let parse_arithmetic = Parser.parse_arithmetic;

let analyze = Analyzer.analyze;

include Framework.NoParseExpression({
  type pp_arg_t = Fmt.t(Result.raw_expression_t);

  type value_t('a) = (
    Operator.Binary.t,
    Expression.expression_t('a),
    Expression.expression_t('a),
  );

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
