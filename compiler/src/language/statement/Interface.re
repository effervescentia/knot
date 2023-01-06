open Knot.Kore;
open AST.Common;

type t('expr, 'typ) =
  | Variable(identifier_t, Node.t('expr, 'typ))
  | Effect(Node.t('expr, 'typ));

type node_t('expr, 'typ) = Node.t(t('expr, 'typ), 'typ);

let fold = (~variable, ~effect) =>
  fun
  | Variable(name, expression) => (name, expression) |> variable
  | Effect(expression) => expression |> effect;
