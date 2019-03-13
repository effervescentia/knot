open Core;
open NestedHashtbl;

let set_pending = ((_, promise)) => {
  switch (promise^) {
  | Unanalyzed => promise := Pending([])
  | _ => ()
  };
  false;
};

let rec resolve = (module_tbl, symbol_tbl) =>
  fun
  | ModuleScope(promise) => Module.resolve(promise) || set_pending(promise)
  | ImportScope(module_, promise) =>
    Import.resolve(module_tbl, symbol_tbl, module_, promise)
    || set_pending(promise)
  | DeclarationScope(name, promise) =>
    Declaration.resolve(symbol_tbl, name, promise) || set_pending(promise)
  | ExpressionScope(promise) =>
    Expression.resolve(promise) || set_pending(promise)
  | ParameterScope(promise) =>
    Property.resolve_param(symbol_tbl, promise) || set_pending(promise)
  | PropertyScope(promise) =>
    Property.resolve(promise) || set_pending(promise)
  | ReferenceScope(promise) =>
    Reference.resolve(symbol_tbl, promise) || set_pending(promise)
  | TypeScope(promise) =>
    Type.resolve(symbol_tbl, promise) || set_pending(promise);
