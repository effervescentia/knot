open Core;
open NestedHashtbl;

exception OperatorTypeMismatch;
exception InvalidDotAccess;
exception ExecutingNonFunction;
exception DefaultValueTypeMismatch;
exception NameInUse(string);

let check_resolution = (resolver, promise, expr) =>
  switch (resolver(promise, expr)) {
  | Some(t) =>
    promise := Resolved(expr, t);
    true;
  | None => false
  };

let typeof =
  fun
  | {contents: Pending(_)} => None
  | {contents: Resolved(_, t)} => Some(t);

let rec resolve = symbol_tbl =>
  fun
  | ModuleScope({contents: Pending(modul)} as promise) =>
    check_resolution(resolve_module, promise, modul)
  | ImportScope({contents: Pending(import)} as promise) => false
  | DeclarationScope({contents: Pending(decl)} as promise) =>
    check_resolution(resolve_decl(symbol_tbl), promise, decl)
  | ExpressionScope({contents: Pending(expr)} as promise) =>
    check_resolution(resolve_expr, promise, expr)
  | ParameterScope({contents: Pending(prop)} as promise) =>
    check_resolution(resolve_param(symbol_tbl), promise, prop)
  | PropertyScope({contents: Pending(prop)} as promise) =>
    check_resolution(resolve_prop, promise, prop)
  | ReferenceScope({contents: Pending(refr)} as promise) =>
    check_resolution(resolve_ref(symbol_tbl), promise, refr)
  | JSXScope({contents: Pending(jsx)} as promise) =>
    check_resolution(resolve_jsx, promise, jsx)
  | _ => false
and resolve_module = promise =>
  fun
  | A_Module(stmts) => None
and resolve_decl = (symbol_tbl, promise) =>
  fun
  | A_ConstDecl(name, expr) =>
    switch (typeof(expr)) {
    | Some(typ) as res =>
      switch (symbol_tbl.find(name)) {
      | Some(_) => raise(NameInUse(name))
      | None =>
        symbol_tbl.add(name, typ);
        res;
      }
    | None => None
    }
  | A_FunctionDecl(name, params, exprs) => {
      let param_types =
        List.map(
          typeof
          % (
            fun
            | Some(typ) => typ
            /* TODO: give these unique IDs within the scope */
            | None => Any_t(0)
          ),
          params,
        );

      if (List.length(exprs) == 0) {
        Some(Function_t(param_types, Nil_t));
      } else {
        switch (typeof(List.nth(exprs, List.length(exprs) - 1))) {
        | Some(t) =>
          let typ = Function_t(param_types, t);
          symbol_tbl.add(name, typ);

          Some(typ);
        | None => None
        };
      };
    }
  | _ => None
and resolve_expr = promise =>
  fun
  | A_BooleanLit(_) => Some(Boolean_t)
  | A_NumericLit(_) => Some(Number_t)
  | A_StringLit(_) => Some(String_t)

  | A_AddExpr(lhs, rhs)
  | A_SubExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Number_t)
    | (Some(String_t), Some(String_t)) => Some(String_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | A_MulExpr(lhs, rhs)
  | A_DivExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Number_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | A_LTExpr(lhs, rhs)
  | A_GTExpr(lhs, rhs)
  | A_LTEExpr(lhs, rhs)
  | A_GTEExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Boolean_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | A_AndExpr(lhs, rhs)
  | A_OrExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Boolean_t), Some(Boolean_t)) => Some(Boolean_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | A_JSX(jsx) => typeof(jsx)
  | A_Reference(refr) => typeof(refr)
and resolve_param = (symbol_tbl, promise, (name, _, _) as param) =>
  resolve_prop(promise, param)
  |> (
    fun
    | Some(typ) as res => {
        symbol_tbl.add(name, typ);

        res;
      }
    | None => None
  )
and resolve_prop = (promise, (name, type_def, default_val)) =>
  switch (type_def, default_val) {
  | (Some(l_type), Some({contents: Resolved(_, r_type)}))
      when l_type !== r_type =>
    raise(DefaultValueTypeMismatch)
  /* TODO: should have a unique ID within scope */
  | (None, None) => Some(Any_t(0))
  | (Some(typ), _)
  | (_, Some({contents: Resolved(_, typ)})) => Some(typ)
  | _ => None
  }
and resolve_ref = (symbol_tbl, promise) =>
  fun
  | A_Variable(name) => symbol_tbl.find(name)
  | A_DotAccess(lhs, rhs) =>
    switch (typeof(lhs), rhs) {
    | (Some(Object_t(props)), name) when Hashtbl.mem(props, name) =>
      Some(Hashtbl.find(props, name))
    | (None, _) => None
    | _ => raise(InvalidDotAccess)
    }
  | A_Execution(refr, args) =>
    switch (typeof(refr)) {
    | Some(Function_t(arg_types, return_type)) => Some(return_type)
    | None => None
    | _ => raise(ExecutingNonFunction)
    }
and resolve_jsx = promise =>
  fun
  | _ => Some(JSX_t);
