open Knot.Kore;

module Plugin =
  AST.Framework.Expression.MakeTypes({
    type parse_arg_t('ast, 'expr) =
      AST.Framework.expression_parser_t('expr, unit);
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) =
      AST.Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr);
    type format_arg_t('expr, 'typ) = 'expr => bool;
    type debug_arg_t('expr, 'typ) = AST.Framework.debug_node_t('expr, 'typ);
    type value_t('expr, 'typ) = Node.t('expr, 'typ);
  });
