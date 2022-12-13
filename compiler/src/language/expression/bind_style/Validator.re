open Knot.Kore;
open AST;

let validate: ((Type.t, Type.t)) => option(Type.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | (Invalid(_), _)
  | (_, Invalid(_)) => None

  | (Valid(`View(_)), Valid(`Style)) => None

  | (view, style) => Some(InvalidStyleBinding(view, style));
