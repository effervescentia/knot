open Core;

type t = {
  resolve: (NestedHashtbl.t(string, member_type), resolve_target) => unit,
};

let of_module = m => ModuleScope(m);
let of_declaration = (name, d) => DeclarationScope(name, d);
let of_import = (module_, i) => ImportScope(module_, i);
let of_parameter = p => ParameterScope(p);
let of_property = p => PropertyScope(p);
let of_expression = e => ExpressionScope(e);
let of_scoped_expression = e => ScopedExpressionScope(e);
let of_reference = r => ReferenceScope(r);
let of_type = t => TypeScope(t);

let (>=>) = (promise, resolver) =>
  if ((snd(promise))^ == None) {
    resolver(promise);
  };

let rec create = module_tbl => {
  resolve: (symbol_tbl, x) => {
    Debug.print_resolve_target(x)
    |> Log.debug("resolving  %s\n%s", Emoji.hourglass_with_flowing_sand);

    resolve(module_tbl, symbol_tbl, x);
  },
}
and resolve = (module_tbl, symbol_tbl) =>
  fun
  | ModuleScope(promise) => promise >=> Module.resolve
  | ImportScope(module_, promise) =>
    promise >=> Import.resolve(module_tbl, symbol_tbl, module_)
  | DeclarationScope(name, promise) =>
    promise >=> Declaration.resolve(symbol_tbl, name)
  | ExpressionScope(promise) => promise >=> Expression.resolve
  | ScopedExpressionScope(promise) =>
    promise >=> Function.resolve_scoped_expr(symbol_tbl)
  | ParameterScope(promise) => promise >=> Property.resolve_param(symbol_tbl)
  | PropertyScope(promise) => promise >=> Property.resolve(symbol_tbl)
  | ReferenceScope(promise) => promise >=> Reference.resolve(symbol_tbl)
  | TypeScope(promise) => promise >=> Type.resolve(symbol_tbl);
