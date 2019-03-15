open Core;
open NestedHashtbl;

let rec resolve = (module_tbl, symbol_tbl) =>
  fun
  | ModuleScope(promise) =>
    Util.is_resolved(promise) || Module.resolve(promise)

  | ImportScope(module_, promise) =>
    Util.is_resolved(promise)
    || Import.resolve(module_tbl, symbol_tbl, module_, promise)

  | DeclarationScope(name, promise) =>
    Util.is_resolved(promise)
    || Declaration.resolve(symbol_tbl, name, promise)

  | ExpressionScope(promise) =>
    Util.is_resolved(promise) || Expression.resolve(promise)

  | ParameterScope(promise) =>
    Util.is_resolved(promise) || Property.resolve_param(symbol_tbl, promise)

  | PropertyScope(promise) =>
    Util.is_resolved(promise) || Property.resolve(promise)

  | ReferenceScope(promise) =>
    Util.is_resolved(promise) || Reference.resolve(symbol_tbl, promise)

  | TypeScope(promise) =>
    Util.is_resolved(promise) || Type.resolve(symbol_tbl, promise);
