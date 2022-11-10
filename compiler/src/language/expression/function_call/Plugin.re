open Knot.Kore;
open AST;

let analyze = Analyzer.analyze_function_call;

include AST.Framework.Expression({
  type parse_arg_t = (
    AST.ParserTypes.expression_parser_t,
    AST.ParserTypes.expression_parser_t,
  );
  type pp_arg_t = Fmt.t(AST.Result.raw_expression_t);

  type value_t('a) = (
    AST.Expression.expression_t('a),
    list(AST.Expression.expression_t('a)),
  );

  let parse = Parser.function_call;

  let pp = Formatter.pp_function_call;

  let to_xml = Debug.to_xml;
});
