open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, name, promise) =>
  fun
  | ConstDecl(expr) =>
    switch (Util.typeof(expr)) {
    | Some(typ) as res =>
      switch (symbol_tbl.find(name)) {
      | Some(_) => raise(NameInUse(name))
      | None =>
        symbol_tbl.add(name, typ);
        res;
      }
    | None => None
    }
  | FunctionDecl(params, exprs) => {
      let param_types =
        List.map(
          Util.typeof
          % (
            fun
            | Some(typ) => typ
            | None => Util.generate_any_type()
          ),
          params,
        );

      if (List.length(exprs) == 0) {
        Some(Function_t(param_types, Nil_t));
      } else {
        switch (Util.typeof(List.nth(exprs, List.length(exprs) - 1))) {
        | Some(t) =>
          let typ = Function_t(param_types, t);
          symbol_tbl.add(name, typ);

          Some(typ);
        | None => None
        };
      };
    }
  | _ => None;
