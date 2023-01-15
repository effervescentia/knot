open Knot.Kore;
open AST.Common;

module StyleRule = {
  type t('expr, 'typ) = (Node.t(string, 'typ), Node.t('expr, 'typ));

  type node_t('expr, 'typ) = raw_t(t('expr, 'typ));
};

module Plugin =
  AST.Framework.Expression.MakeTypes({
    type parse_arg_t('ast, 'expr) = (
      AST.ParseContext.t('ast),
      AST.Framework.Interface.contextual_parse_t('ast, 'expr),
    );
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) =
      AST.Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr);
    type format_arg_t('expr, 'typ) = unit;
    type value_t('expr, 'typ) = list(StyleRule.node_t('expr, 'typ));
  });
