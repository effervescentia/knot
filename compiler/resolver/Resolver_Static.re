open Core;
open NestedHashtbl;

let is_resolved = ((_, promise)) =>
  switch (promise^ ^) {
  | Resolved(_) => true
  | _ => false
  };

let rec resolve = (module_tbl, symbol_tbl) =>
  fun
  | ModuleScope(promise) => is_resolved(promise) || Module.resolve(promise)

  | ImportScope(module_, promise) =>
    is_resolved(promise)
    || Import.resolve(module_tbl, symbol_tbl, module_, promise)

  | DeclarationScope(name, promise) =>
    is_resolved(promise) || Declaration.resolve(symbol_tbl, name, promise)

  | ExpressionScope(promise) =>
    is_resolved(promise) || Expression.resolve(promise)

  | ParameterScope(promise) =>
    is_resolved(promise) || Property.resolve_param(symbol_tbl, promise)

  | PropertyScope(promise) =>
    is_resolved(promise) || Property.resolve(promise)

  | ReferenceScope(promise) =>
    is_resolved(promise) || Reference.resolve(symbol_tbl, promise)

  | TypeScope(promise) =>
    is_resolved(promise) || Type.resolve(symbol_tbl, promise);
