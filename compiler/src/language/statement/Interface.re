open Knot.Kore;
open AST.Common;

type t('expr) =
  | Variable(identifier_t, 'expr)
  | Effect('expr);

type node_t('expr, 'typ) = Node.t(t('expr), 'typ);

/* helpers */

let variable = ((name, expression)) => Variable(name, expression);
let effect = expression => Effect(expression);
