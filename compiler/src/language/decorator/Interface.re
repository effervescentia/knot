open Knot.Kore;
open AST.Common;

module Plugin =
  AST.Framework.ParsePlugin.Make({
    type value_t('expr, 'typ) = (identifier_t, list(Node.t('expr, 'typ)));
    type parse_arg_t('ast, 'expr) = AST.Framework.Interface.parse_t('expr);
  });
