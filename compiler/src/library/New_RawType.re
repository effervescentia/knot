open Infix;
open Reference;
open New_Type;

type type_err = |;

type t =
  | Strong(strong_t)
  | Weak(ref(result(weak_t, type_err)))
  | Invalid(type_err)

and strong_t = [ primitive_t | container_t(t)]

and weak_t = [ primitive_t | container_t(t) | abstract_t(trait_t)];
