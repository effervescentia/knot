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
  | {contents: Resolved(_, t)} => Some(t)
  | _ => None;

let rec resolve = symbol_tbl =>
  fun
  | ModuleScope({contents: Pending(modul, _)} as promise) =>
    check_resolution(resolve_module, promise, modul)
  | ImportScope({contents: Pending(import, _)} as promise) => false
  | DeclarationScope({contents: Pending(decl, _)} as promise) =>
    check_resolution(resolve_decl(symbol_tbl), promise, decl)
  | ExpressionScope({contents: Pending(expr, _)} as promise) =>
    check_resolution(resolve_expr, promise, expr)
  | ParameterScope({contents: Pending(prop, _)} as promise) =>
    check_resolution(resolve_param(symbol_tbl), promise, prop)
  | PropertyScope({contents: Pending(prop, _)} as promise) =>
    check_resolution(resolve_prop, promise, prop)
  | ReferenceScope({contents: Pending(refr, _)} as promise) =>
    check_resolution(resolve_ref(symbol_tbl), promise, refr)
  | _ => false
and resolve_module = promise =>
  fun
  | Module(stmts) => None
and resolve_decl = (symbol_tbl, promise) =>
  fun
  | ConstDecl(name, expr) =>
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
  | FunctionDecl(name, params, exprs) => {
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
  | BooleanLit(_) => Some(Boolean_t)
  | NumericLit(_) => Some(Number_t)
  | StringLit(_) => Some(String_t)
  | JSX(jsx) => Some(JSX_t)

  | AddExpr(lhs, rhs)
  | SubExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Number_t)
    | (Some(String_t), Some(String_t)) => Some(String_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | MulExpr(lhs, rhs)
  | DivExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Number_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | LTExpr(lhs, rhs)
  | GTExpr(lhs, rhs)
  | LTEExpr(lhs, rhs)
  | GTEExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Boolean_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | AndExpr(lhs, rhs)
  | OrExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Boolean_t), Some(Boolean_t)) => Some(Boolean_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | Reference(refr) => typeof(refr)
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
  | (
      Some({contents: Resolved(_, l_type)}),
      Some({contents: Resolved(_, r_type)}),
    )
      when l_type !== r_type =>
    raise(DefaultValueTypeMismatch)
  /* TODO: should have a unique ID within scope */
  | (None, None) => Some(Any_t(0))
  | (Some({contents: Resolved(_, typ)}), _)
  | (_, Some({contents: Resolved(_, typ)})) => Some(typ)
  | _ => None
  }
and resolve_ref = (symbol_tbl, promise) =>
  fun
  | Variable(name) => symbol_tbl.find(name)
  | DotAccess(lhs, rhs) =>
    switch (typeof(lhs), rhs) {
    | (Some(Object_t(props)), name) when Hashtbl.mem(props, name) =>
      Some(Hashtbl.find(props, name))
    | (None, _) => None
    | _ => raise(InvalidDotAccess)
    }
  | Execution(refr, args) =>
    switch (typeof(refr)) {
    | Some(Function_t(arg_types, return_type)) => Some(return_type)
    | None => None
    | _ => raise(ExecutingNonFunction)
    };
