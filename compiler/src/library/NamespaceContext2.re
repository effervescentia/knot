open Infix;
open Reference;

type t = {
  /* unique identifier for this namespace */
  namespace: Namespace.t,
  /* error reporting callback */
  report: Error.compile_err => unit,
  /* error reporting callback */
  mutable inner_modules: list((Module.t, Hashtbl.t(Export.t, Type.t))),
};
