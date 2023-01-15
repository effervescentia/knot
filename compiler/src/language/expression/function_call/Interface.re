open Knot.Kore;

module Plugin =
  AST.Framework.Expression.MakeTypes({
    type parse_arg_t('ast, 'expr) = (
      AST.Framework.Interface.parse_t('expr),
      AST.Framework.Interface.parse_t('expr),
    );
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) =
      AST.Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr);
    type format_arg_t('expr, 'typ) = unit;
    type value_t('expr, 'typ) = (
      Node.t('expr, 'typ),
      list(Node.t('expr, 'typ)),
    );
  });
