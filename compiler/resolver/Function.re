open Core;

let resolve_callable = (params, exprs) => {
  let param_types = List.map(opt_type_ref, params);
  let return_type =
    if (List.length(exprs) == 0) {
      Nil_t;
    } else {
      List.nth(exprs, List.length(exprs) - 1) |> opt_type_ref;
    };

  (param_types, return_type);
};

let resolve_scoped_expr = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | ExpressionStatement(expr) => opt_type_ref(expr)
    | VariableDeclaration(name, expr) =>
      switch (NestedHashtbl.find_local(symbol_tbl, name)) {
      | Some(_) => throw_semantic(NameInUse(name))
      | None =>
        opt_type_ref(expr) |> NestedHashtbl.add(symbol_tbl, name);

        Nil_t;
      }
    | VariableAssignment(refr, expr) =>
      if (opt_type_ref(refr) != opt_type_ref(expr)) {
        throw_semantic(OperatorTypeMismatch);
      };

      Nil_t;
    }
  )
  <:= promise;
