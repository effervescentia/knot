open Core;
open NestedHashtbl;

let set_pending = promise =>
  switch (promise^) {
  | Unanalyzed => promise := Pending([])
  | _ => ()
  };

let check_resolution = (resolver, (expr, promise)) =>
  switch (resolver(promise, expr)) {
  | Some(t) =>
    promise := Resolved(t);
    true;
  | None =>
    set_pending(promise);
    false;
  };

let rec resolve = (module_tbl, symbol_tbl) =>
  fun
  | ModuleScope(promise) => check_resolution(Module.resolve, promise)
  | ImportScope(module_, promise) =>
    check_resolution(
      Import.resolve(module_tbl, symbol_tbl, module_),
      promise,
    )
  | DeclarationScope(name, promise) =>
    check_resolution(Declaration.resolve(symbol_tbl, name), promise)
  | ExpressionScope(promise) => check_resolution(Expression.resolve, promise)
  | ParameterScope(promise) =>
    check_resolution(Property.resolve_param(symbol_tbl), promise)
  | PropertyScope(promise) => check_resolution(Property.resolve, promise)
  | ReferenceScope(promise) =>
    check_resolution(Reference.resolve(symbol_tbl), promise)
  | TypeScope(promise) =>
    check_resolution(resolve_type(symbol_tbl), promise)
and resolve_type = (symbol_tbl, promise) =>
  fun
  | "string" => Some(String_t)
  | "number" => Some(Number_t)
  | "boolean" => Some(Number_t)
  | _ => raise(InvalidTypeReference);
