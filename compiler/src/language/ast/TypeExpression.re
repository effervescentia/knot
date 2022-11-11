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
  | Struct(list((untyped_t(string), t)))
  | Function(list(t), t)
  | DotAccess(t, untyped_t(string))
  | View(t, t);

/* tag helpers */

let of_id = x => Identifier(x);
let of_group = x => Group(x);
let of_list = x => List(x);
let of_struct = props => Struct(props);
let of_function = ((args, res)) => Function(args, res);
let of_dot_access = ((id, prop)) => DotAccess(id, prop);
let of_view = ((props, res)) => View(props, res);
