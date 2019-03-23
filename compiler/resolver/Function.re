open Core;
open NestedHashtbl;

let resolve_scoped_expr = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | ExpressionStatement(expr) => opt_type_ref(expr)
    | VariableDeclaration(name, expr) =>
      switch (symbol_tbl.find_local(name)) {
      | Some(_) => raise(NameInUse(name))
      | None =>
        opt_type_ref(expr) |> symbol_tbl.add(name);

        declared(Nil_t);
      }
    }
  )
  <:= promise;
