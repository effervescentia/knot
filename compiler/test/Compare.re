open Knot.Core;

let as_t = x => ref(Some(x));

let rec compare_types = (lhs, rhs) =>
  switch (lhs^, rhs^) {
  | (Some(lhs), Some(rhs)) =>
    switch (fst(lhs^), fst(rhs^)) {
    | (Array_t(lhs), Array_t(rhs)) => compare_types(as_t(lhs), as_t(rhs))
    | (Function_t(lhs_args, lhs_ret), Function_t(rhs_args, rhs_ret))
    | (View_t(lhs_args, lhs_ret), View_t(rhs_args, rhs_ret)) =>
      List.for_all2(
        compare_types,
        List.map(x => as_t(x), lhs_args),
        List.map(x => as_t(x), rhs_args),
      )
      && compare_types(as_t(lhs_ret), as_t(rhs_ret))
    | (Object_t(lhs_tbl), Object_t(rhs_tbl)) =>
      compare_type_tbls(lhs_tbl, rhs_tbl)
    | (
        Module_t(lhs_imports, lhs_members, lhs_main),
        Module_t(rhs_imports, rhs_members, rhs_main),
      ) =>
      lhs_imports == rhs_imports
      && compare_type_tbls(lhs_members, rhs_members)
      && (
        switch (lhs_main, rhs_main) {
        | (Some(lhs), Some(rhs)) => compare_types(as_t(lhs), as_t(rhs))
        | (None, None) => true
        | _ => false
        }
      )
    | (Generic_t(_), Generic_t(_)) => raise(NotImplemented)
    | (lhs, rhs) => lhs == rhs
    }
  | (None, None) => true
  | _ => false
  }
and compare_type_tbls = (lhs, rhs) =>
  Hashtbl.length(lhs) == Hashtbl.length(rhs)
  && Hashtbl.to_seq_keys(lhs)
  |> Seq.fold_left(
       (matches, key) =>
         matches
         && Hashtbl.mem(rhs, key)
         && compare_types(
              as_t(Hashtbl.find(lhs, key)),
              as_t(Hashtbl.find(rhs, key)),
            ),
       true,
     );

let rec compare_modules = ((lhs, lhs_t), (rhs, rhs_t)) =>
  compare_types(lhs_t, rhs_t)
  && (
    switch (lhs, rhs) {
    | (Module(lhs_stmts), Module(rhs_stmts)) =>
      List.for_all2(compare_stmts, lhs_stmts, rhs_stmts)
    }
  )
and compare_stmts = (lhs, rhs) =>
  switch (lhs, rhs) {
  | (Declaration(lhs_name, lhs_decl), Declaration(rhs_name, rhs_decl)) =>
    lhs_name == rhs_name && compare_decls(lhs_decl, rhs_decl)
  | _ => raise(NotImplemented)
  }
and compare_decls = ((lhs, lhs_t), (rhs, rhs_t)) =>
  compare_types(lhs_t, rhs_t)
  && (
    switch (lhs, rhs) {
    | (ConstDecl(lhs), ConstDecl(rhs)) => compare_exprs(lhs, rhs)
    | (
        FunctionDecl(lhs_props, lhs_exprs),
        FunctionDecl(rhs_props, rhs_exprs),
      ) =>
      List.for_all2(compare_props, lhs_props, rhs_props)
      && List.for_all2(compare_scoped_exprs, lhs_exprs, rhs_exprs)
    | (
        ViewDecl(_, _, lhs_props, lhs_exprs),
        ViewDecl(_, _, rhs_props, rhs_exprs),
      ) =>
      List.for_all2(compare_props, lhs_props, rhs_props)
      && List.for_all2(compare_scoped_exprs, lhs_exprs, rhs_exprs)
    | (StateDecl(_), StateDecl(_)) => raise(NotImplemented)
    | (StyleDecl(_), StyleDecl(_)) => raise(NotImplemented)
    | _ => false
    }
  )
and compare_props = ((lhs, lhs_t), (rhs, rhs_t)) =>
  compare_types(lhs_t, rhs_t)
  && (
    switch (lhs, rhs) {
    | ((lhs_name, lhs_type, lhs_expr), (rhs_name, rhs_type, rhs_expr)) =>
      lhs_name == rhs_name
      && lhs_type == rhs_type
      && (
        switch (lhs_expr, rhs_expr) {
        | (Some(lhs), Some(rhs)) => compare_exprs(lhs, rhs)
        | (None, None) => true
        | _ => false
        }
      )
    }
  )
and compare_scoped_exprs = ((lhs, lhs_t), (rhs, rhs_t)) =>
  compare_types(lhs_t, rhs_t)
  && (
    switch (lhs, rhs) {
    | (ExpressionStatement(lhs), ExpressionStatement(rhs)) =>
      compare_exprs(lhs, rhs)
    | (
        VariableDeclaration(lhs_name, lhs_expr),
        VariableDeclaration(rhs_name, rhs_expr),
      ) =>
      lhs_name == rhs_name && compare_exprs(lhs_expr, rhs_expr)
    | _ => false
    }
  )
and compare_exprs = ((lhs, lhs_t), (rhs, rhs_t)) =>
  compare_types(lhs_t, rhs_t)
  && (
    switch (lhs, rhs) {
    | (AddExpr(lhs_a, lhs_b), AddExpr(rhs_a, rhs_b))
    | (SubExpr(lhs_a, lhs_b), SubExpr(rhs_a, rhs_b))
    | (MulExpr(lhs_a, lhs_b), MulExpr(rhs_a, rhs_b))
    | (DivExpr(lhs_a, lhs_b), DivExpr(rhs_a, rhs_b))
    | (LTExpr(lhs_a, lhs_b), LTExpr(rhs_a, rhs_b))
    | (GTExpr(lhs_a, lhs_b), GTExpr(rhs_a, rhs_b))
    | (LTEExpr(lhs_a, lhs_b), LTEExpr(rhs_a, rhs_b))
    | (GTEExpr(lhs_a, lhs_b), GTEExpr(rhs_a, rhs_b))
    | (AndExpr(lhs_a, lhs_b), AndExpr(rhs_a, rhs_b))
    | (OrExpr(lhs_a, lhs_b), OrExpr(rhs_a, rhs_b)) =>
      compare_exprs(lhs_a, rhs_a) && compare_exprs(lhs_b, rhs_b)
    | (TernaryExpr(lhs_a, lhs_b, lhs_c), TernaryExpr(rhs_a, rhs_b, rhs_c)) =>
      compare_exprs(lhs_a, rhs_a)
      && compare_exprs(lhs_b, rhs_b)
      && compare_exprs(lhs_c, rhs_c)
    | (Reference(lhs), Reference(rhs)) => compare_refs(lhs, rhs)
    | (JSX(lhs), JSX(rhs)) => lhs == rhs
    | (NumericLit(lhs), NumericLit(rhs)) => lhs == rhs
    | (BooleanLit(lhs), BooleanLit(rhs)) => lhs == rhs
    | (StringLit(lhs), StringLit(rhs)) => lhs == rhs
    | _ => false
    }
  )
and compare_refs = ((lhs, lhs_t), (rhs, rhs_t)) =>
  compare_types(lhs_t, rhs_t)
  && (
    switch (lhs, rhs) {
    | (Variable(lhs), Variable(rhs)) => lhs == rhs
    | (DotAccess(lhs_var, lhs_prop), DotAccess(rhs_var, rhs_prop)) =>
      compare_refs(lhs_var, rhs_var) && lhs_prop == rhs_prop
    | (Execution(lhs_var, lhs_exprs), Execution(rhs_var, rhs_exprs)) =>
      compare_refs(lhs_var, rhs_var)
      && List.for_all2(compare_exprs, lhs_exprs, rhs_exprs)
    | _ => false
    }
  );
