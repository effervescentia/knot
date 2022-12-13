/**
 Type expression Abstract Syntax Tree.
 */
open Common;

type t = untyped_t(raw_t)

and raw_t =
  | Nil
  | Boolean
  | Integer
  | Float
  | String
  | Element
  | Style
  | Identifier(untyped_t(string))
  | Group(t)
  | List(t)
  | Struct(list(struct_entry_t))
  | Function(list(t), t)
  | DotAccess(t, untyped_t(string))
  | View(t, t)

and struct_entry_t = untyped_t(raw_struct_entry_t)

and raw_struct_entry_t =
  | Required(untyped_t(string), t)
  | Optional(untyped_t(string), t)
  | Spread(t);

/* tag helpers */

let of_id = x => Identifier(x);
let of_group = x => Group(x);
let of_list = x => List(x);
let of_struct = props => Struct(props);
let of_function = ((args, res)) => Function(args, res);
let of_dot_access = ((id, prop)) => DotAccess(id, prop);
let of_view = ((props, res)) => View(props, res);

let of_required = ((key, value)) => Required(key, value);
let of_optional = ((key, value)) => Optional(key, value);
let of_spread = value => Spread(value);
