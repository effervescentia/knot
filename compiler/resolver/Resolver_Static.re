open Core;
open NestedHashtbl;

let set_pending = ((_, promise)) => {
  switch (promise^ ^) {
  | Unanalyzed => promise := ref(Pending([]))
  | _ => ()
  };
  false;
};

let is_resolved = ((_, promise)) =>
  switch (promise^ ^) {
  | Resolved(_) => true
  | _ => false
  };

let rec resolve = (module_tbl, symbol_tbl) =>
  fun
  | ModuleScope(promise) =>
    is_resolved(promise) || Module.resolve(promise) || set_pending(promise)
  | ImportScope(module_, promise) =>
    is_resolved(promise)
    || Import.resolve(module_tbl, symbol_tbl, module_, promise)
    || set_pending(promise)
  | DeclarationScope(name, promise) =>
    is_resolved(promise)
    || Declaration.resolve(symbol_tbl, name, promise)
    || set_pending(promise)
  | ExpressionScope(promise) =>
    is_resolved(promise)
    || Expression.resolve(promise)
    || set_pending(promise)
  | ParameterScope(promise) =>
    is_resolved(promise)
    || Property.resolve_param(symbol_tbl, promise)
    || set_pending(promise)
  | PropertyScope(promise) =>
    is_resolved(promise)
    || Property.resolve(promise)
    || set_pending(promise)
  | ReferenceScope(promise) =>
    is_resolved(promise)
    || Reference.resolve(symbol_tbl, promise)
    || set_pending(promise)
  | TypeScope(promise) =>
    is_resolved(promise)
    || Type.resolve(symbol_tbl, promise)
    || set_pending(promise);
