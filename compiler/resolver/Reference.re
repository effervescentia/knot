open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, promise) =>
  (
    switch (fst(promise)) {
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
      | Some(Any_t) as res => res
      | None => None
      | _ => raise(ExecutingNonFunction)
      }
    }
  )
  |%> resolve_iff(promise);
