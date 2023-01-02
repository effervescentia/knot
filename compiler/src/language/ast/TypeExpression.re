/**
 Type expression Abstract Syntax Tree.
 */
open Common;

type t = Common.raw_t(raw_t)

and raw_t =
  | Nil
  | Boolean
  | Integer
  | Float
  | String
  | Element
  | Style
  | Identifier(identifier_t)
  | Group(t)
  | List(t)
  | Object(list(object_entry_t))
  | Function(list(t), t)
  | DotAccess(t, identifier_t)
  | View(t, t)

and object_entry_t = Common.raw_t(raw_object_entry_t)

and raw_object_entry_t =
  | Required(identifier_t, t)
  | Optional(identifier_t, t)
  | Spread(t);

/* tag helpers */

let of_id = x => Identifier(x);
let of_group = x => Group(x);
let of_list = x => List(x);
let of_object = props => Object(props);
let of_function = ((args, res)) => Function(args, res);
let of_dot_access = ((id, prop)) => DotAccess(id, prop);
let of_view = ((props, res)) => View(props, res);

let of_required = ((key, value)) => Required(key, value);
let of_optional = ((key, value)) => Optional(key, value);
let of_spread = value => Spread(value);
