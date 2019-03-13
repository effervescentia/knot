open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, promise) =>
  fun
  | Variable(name) => symbol_tbl.find(name)
  | DotAccess(lhs, rhs) =>
    switch (Util.typeof(lhs), rhs) {
    | (Some(Object_t(props)), name) when Hashtbl.mem(props, name) =>
      Some(Hashtbl.find(props, name))
    | (None, _) => None
    | _ => raise(InvalidDotAccess)
    }
  | Execution(refr, args) =>
    switch (Util.typeof(refr)) {
    | Some(Function_t(arg_types, return_type)) => Some(return_type)
    /* TODO: handle this properly */
    | Some(Any_t(_)) => Some(Util.generate_any_type())
    | None => None
    | _ => raise(ExecutingNonFunction)
    };
