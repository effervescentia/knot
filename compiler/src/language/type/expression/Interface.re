open Knot.Kore;

module ObjectEntry = {
  type t =
    | Required(AST.Common.identifier_t, t)
    | Optional(AST.Common.identifier_t, t)
    | Spread(t);

  type node_t = AST.Common.raw_t(t);
};

type t =
  | Nil
  | Boolean
  | Integer
  | Float
  | String
  | Element
  | Style
  | Identifier(AST.Common.identifier_t)
  | Group(node_t)
  | List(node_t)
  | Object(list(ObjectEntry.node_t))
  | Function(list(node_t), node_t)
  | DotAccess(node_t, AST.Common.identifier_t)
  | View(node_t, node_t)

and node_t = AST.Common.raw_t(t);

// /* tag helpers */

// let of_id = x => Identifier(x);
// let of_group = x => Group(x);
// let of_list = x => List(x);
// let of_object = props => Object(props);
// let of_function = ((args, res)) => Function(args, res);
// let of_dot_access = ((id, prop)) => DotAccess(id, prop);
// let of_view = ((props, res)) => View(props, res);

// let of_required = ((key, value)) => Required(key, value);
// let of_optional = ((key, value)) => Optional(key, value);
// let of_spread = value => Spread(value);
