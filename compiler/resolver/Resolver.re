open Core;

type t = {resolve: (context_t, resolve_target) => unit}
and context_t = {
  symbol_tbl: NestedHashtbl.t(string, member_type),
  sidecar: option(Hashtbl.t(string, member_type)),
};

let of_module = m => ModuleScope(m);
let of_declaration = (name, d) => DeclarationScope(name, d);
let of_import = (module_, i) => ImportScope(module_, i);
let of_parameter = p => ParameterScope(p);
let of_property = p => PropertyScope(p);
let of_state_property = (name, p) => StatePropertyScope(name, p);
let of_state_method = (name, m) => StateMethodScope(name, m);
let of_expression = e => ExpressionScope(e);
let of_scoped_expression = e => ScopedExpressionScope(e);
let of_reference = r => ReferenceScope(r);
let of_type = t => TypeScope(t);
let of_mixin = t => MixinScope(t);

let (>=>) = (promise, resolver) =>
  if ((snd(promise))^ == None) {
    resolver(promise);
  };

let rec create = module_tbl => {
  resolve: (ctx, x) => {
    Debug.print_resolve_target(x)
    |> Log.debug("resolving  %s\n%s", Emoji.hourglass_with_flowing_sand);

    resolve(module_tbl, ctx, x);
  },
}
and resolve = (module_tbl, {symbol_tbl, sidecar}) =>
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
  | StatePropertyScope(name, promise) =>
    switch (sidecar) {
    | Some(x) => promise >=> State.resolve_prop(x, name)
    | None => throw(MissingSidecarScope)
    }
  | StateMethodScope(name, promise) =>
    switch (sidecar) {
    | Some(x) => promise >=> State.resolve_method(x, name)
    | None => throw(MissingSidecarScope)
    }
  | ReferenceScope(promise) =>
    promise >=> Reference.resolve(symbol_tbl, sidecar)
  | TypeScope(promise) => promise >=> Type.resolve(symbol_tbl)
  | MixinScope(promise) =>
    switch (sidecar) {
    | Some(x) => promise >=> Type.resolve_mixin(symbol_tbl, x)
    | None => throw(MissingSidecarScope)
    };
