open Core;
open NestedHashtbl;

exception OperatorTypeMismatch;
exception InvalidDotAccess;
exception ExecutingNonFunction;
exception DefaultValueTypeMismatch;
exception InvalidTypeReference;
exception NameInUse(string);

/* TODO: give these unique IDs within the scope */
let generate_any_type = () => Any_t(0);

let check_resolution = (resolver, promise) => {
  let expr = abandon_ctx(promise);

  switch (resolver(promise, expr)) {
  | Some(t) =>
    promise := Resolved(expr, t);
    true;
  | None => false
  };
};

let typeof =
  fun
  | {contents: Resolved(_, t)} => Some(t)
  | _ => None;

let is_resolved = target =>
  typeof(target)
  |> (
    fun
    | Some(_) => true
    | None => false
  );

let rec resolve = (module_tbl, symbol_tbl) =>
  fun
  | ModuleScope(promise) => check_resolution(resolve_module, promise)
  | ImportScope(module_, promise) =>
    check_resolution(
      resolve_import(module_tbl, symbol_tbl, module_),
      promise,
    )
  | DeclarationScope(promise) =>
    check_resolution(resolve_decl(symbol_tbl), promise)
  | ExpressionScope(promise) => check_resolution(resolve_expr, promise)
  | ParameterScope(promise) =>
    check_resolution(resolve_param(symbol_tbl), promise)
  | PropertyScope(promise) => check_resolution(resolve_prop, promise)
  | ReferenceScope(promise) =>
    check_resolution(resolve_ref(symbol_tbl), promise)
  | TypeScope(promise) =>
    check_resolution(resolve_type(symbol_tbl), promise)
and resolve_type = (symbol_tbl, promise) =>
  fun
  | "string" => Some(String_t)
  | "number" => Some(Number_t)
  | "boolean" => Some(Number_t)
  | _ => raise(InvalidTypeReference)
and resolve_module = promise =>
  fun
  | Module(stmts) => {
      let dependencies = ref([]);
      let declarations = Hashtbl.create(8);
      let main_declaration = ref(None);

      if (List.for_all(
            fun
            | Import(module_, imports) => {
                dependencies := [module_, ...dependencies^];

                List.for_all(is_resolved, imports);
              }
            | Declaration(decl) =>
              switch (typeof(decl)) {
              | Some(typ) =>
                Hashtbl.add(
                  declarations,
                  abandon_ctx(decl) |> Util.extract_decl_name,
                  typ,
                );

                true;
              | None => false
              }
            | Main(decl) =>
              switch (typeof(decl)) {
              | Some(typ) =>
                Hashtbl.add(
                  declarations,
                  abandon_ctx(decl) |> Util.extract_decl_name,
                  typ,
                );
                main_declaration := Some(typ);

                true;
              | None => false
              },
            stmts,
          )) {
        Some(Module_t(dependencies^, declarations, main_declaration^));
      } else {
        None;
      };
    }
and resolve_import = (module_tbl, symbol_tbl, module_, promise) =>
  fun
  | ModuleExport(name) => {
      symbol_tbl.add(name, generate_any_type());

      None;
    }
  | MainExport(name) => {
      symbol_tbl.add(name, generate_any_type());

      None;
    }
  | NamedExport(name, alias) => {
      (
        switch (alias) {
        | Some(s) => s
        | None => name
        }
      )
      |> (s => symbol_tbl.add(s, generate_any_type()));

      None;
    }
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
            | None => generate_any_type()
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
  | (None, None) => Some(generate_any_type())
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
    /* TODO: handle this properly */
    | Some(Any_t(_)) => Some(generate_any_type())
    | None => None
    | _ => raise(ExecutingNonFunction)
    };
