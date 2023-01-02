open Knot.Kore;
open Common;

module ObjectEntry = {
  type t('expr) =
    | Required(identifier_t, 'expr)
    | Optional(identifier_t, 'expr)
    | Spread('expr);

  type node_t('expr) = raw_t(t('expr));
};

type t =
  | Nil
  | Boolean
  | Integer
  | Float
  | String
  | Style
  | Element
  | Identifier(string)
  | Group(node_t)
  | List(node_t)
  | Object(list(ObjectEntry.node_t(node_t)))
  | Function(list(node_t), node_t)
  | DotAccess(node_t, identifier_t)
  | View(node_t, node_t)

and node_t = raw_t(t);
