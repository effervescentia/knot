open Knot.Kore;

/* plugin types */

module Plugin =
  AST.Framework.Expression.MakeTypes({
    type parse_arg_t('ast, 'expr) = (
      AST.ParseContext.t('ast),
      (
        /* parses a view expression */
        AST.Framework.Interface.contextual_parse_t('ast, 'expr),
        /* parses a style literal */
        AST.Framework.Interface.contextual_parse_t('ast, 'expr),
      ),
    );

    type analyze_arg_t('ast, 'raw_expr, 'result_expr) = (
      AST.Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr),
      ('raw_expr => option(string), string => 'result_expr),
    );

    type format_arg_t('expr, 'typ) =
      'expr => option(list(KStyle.Interface.StyleRule.node_t('expr, 'typ)));

    type value_t('expr, 'typ) = (
      KSX.Interface.ViewKind.t,
      Node.t('expr, 'typ),
      Node.t('expr, 'typ),
    );
  });
