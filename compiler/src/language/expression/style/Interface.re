open Knot.Kore;
open AST.Common;

module StyleRule = {
  type t('expr, 'typ) = (Node.t(string, 'typ), Node.t('expr, 'typ));

  type node_t('expr, 'typ) = raw_t(t('expr, 'typ));
};
