open Knot.Kore;

module Plugin =
  AST.Framework.NoParseExpression.MakeTypes({
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) =
      AST.Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr);
    type format_arg_t('expr, 'typ) = unit;
    type value_t('expr, 'typ) = (
      AST.Operator.Binary.t,
      Node.t('expr, 'typ),
      Node.t('expr, 'typ),
    );
  });
