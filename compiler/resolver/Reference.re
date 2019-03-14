open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, promise) =>
  switch (fst(promise)) {
  | Variable(name) =>
    switch (symbol_tbl.find(name)) {
    | None =>
      let typ = any_cast();
      snd(promise) := ref(typ);

      true;
    /* symbol_tbl.add(name, snd(promise)s); */
    /* | Some({contents: {contents: Synthetic(typ, _)}} as res) =>
         /* do type inference here */
         true
       | Some({contents: {contents: Resolved(_)} as res}) =>
         snd(promise) := res;

         true; */
    | _ => false
    }
  | DotAccess(lhs, rhs) =>
    switch (Util.typeof(lhs), rhs) {
    | (Some(Object_t(props)), name) when Hashtbl.mem(props, name) =>
      snd(promise) := ref(Resolved(Hashtbl.find(props, name)));
      true;
    | (None, _) => false
    | _ => raise(InvalidDotAccess)
    }
  | Execution(refr, args) =>
    switch (Util.typeof(refr)) {
    | Some(Function_t(arg_types, return_type)) =>
      snd(promise) := ref(Resolved(return_type));

      true;
    /* TODO: handle this properly */
    | Some(Any_t) =>
      snd(promise) := (snd(refr))^;
      true;
    | None => false
    | _ => raise(ExecutingNonFunction)
    }
  };
