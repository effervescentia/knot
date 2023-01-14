open Knot.Kore;

module Plugin =
  AST.Framework.Statement.MakeTypes({
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) =
      AST.Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr);
    type value_t('expr, 'typ) = Node.t('expr, 'typ);
  });
