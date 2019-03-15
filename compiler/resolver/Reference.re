open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | Variable(name) =>
      switch (symbol_tbl.find(name)) {
      | None =>
        let typ = any_cast();
        Some(ref(typ));
      /* symbol_tbl.add(name, snd(promise)s); */
      /* | Some({contents: {contents: Synthetic(typ, _)}} as res) =>
           /* do type inference here */
           true
         | Some({contents: {contents: Resolved(_)} as res}) =>
           snd(promise) := res;

           true; */
      | _ => None
      }
    | DotAccess(lhs, rhs) =>
      switch (Util.typeof(lhs), rhs) {
      | (Some(Object_t(props)), name) when Hashtbl.mem(props, name) =>
        Some(ref(Resolved(Hashtbl.find(props, name))))
      | (None, _) => None
      | _ => raise(InvalidDotAccess)
      }
    | Execution(refr, args) =>
      switch (Util.typeof(refr)) {
      | Some(Function_t(arg_types, return_type)) =>
        Some(ref(Resolved(return_type)))
      /* TODO: handle this properly */
      | Some(Any_t) => Some((snd(refr))^)
      | None => None
      | _ => raise(ExecutingNonFunction)
      }
    }
  )
  |::> promise;
