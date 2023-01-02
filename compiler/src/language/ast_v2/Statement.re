open Knot.Kore;
open Common;

type t('expr) =
  | Variable(identifier_t, 'expr)
  | Effect('expr);

type node_t('expr, 'typ) = Node.t(t('expr), 'typ);
