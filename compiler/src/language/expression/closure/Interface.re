open Knot.Kore;

module Plugin =
  AST.Framework.Expression.MakeTypes({
    type parse_arg_t('ast, 'expr) = (
      AST.ParseContext.t('ast),
      AST.Framework.Interface.contextual_parse_t('ast, 'expr),
    );
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) =
      AST.Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr);
    type format_arg_t('expr, 'typ) = unit;
    type value_t('expr, 'typ) =
      list(KStatement.Interface.node_t('expr, 'typ));
  });
