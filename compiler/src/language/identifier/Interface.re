open Knot.Kore;

module Plugin =
  AST.Framework.Expression.MakeTypes({
    type parse_arg_t('ast, 'expr) = AST.ParseContext.t('ast);
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) = unit;
    type format_arg_t('expr, 'typ) = unit;
    type value_t('expr, 'typ) = string;
  });
