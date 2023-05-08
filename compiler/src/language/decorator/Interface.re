open Knot.Kore;
open AST.Common;

type t('expr, 'typ) = (Node.t(string, 'typ), list(Node.t('expr, 'typ)));

module Plugin = {
  include AST.Framework.ParsePlugin.Make({
    type value_t('expr, 'typ) = t('expr, 'typ);
    type parse_arg_t('ast, 'expr) = AST.Framework.Interface.parse_t('expr);
  });

  include AST.Framework.DebugPlugin.Make({
    type value_t('expr, 'typ) = raw_t(t('expr, 'typ));
    type debug_arg_t('expr, 'typ) = (
      AST.Framework.Interface.debug_t(Node.t('expr, 'typ)),
      'typ => string,
    );
  });
};
