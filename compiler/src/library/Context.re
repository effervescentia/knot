open Infix;
open Reference;

/**
 compilation context used for reporting errors and accessing scope
 */
type t = {
  namespace: Namespace.t,
  scope: Scope.t,
  report: Error.compile_err => unit,
};

/* static */

let create =
    (~scope=Scope.create(), ~report=Error.throw, namespace: Namespace.t) => {
  namespace,
  scope,
  report,
};

/* methods */

let clone = (ctx: t) => {...ctx, scope: ctx.scope |> Scope.clone};

/**
 find the type of an export from a different module and import it into the current scope
 */
let import =
    (
      namespace: Namespace.t,
      (id, cursor): (Export.t, Cursor.t),
      label: Identifier.t,
      ctx: t,
    ) => {
  let type_ =
    switch (ctx.scope |> Scope.lookup(namespace, id)) {
    | Ok(t) => t
    | Error(err) =>
      ctx.report(ParseError(TypeError(err), ctx.namespace, cursor));
      Type.K_Invalid(err);
    };

  Scope.define(label, type_, ctx.scope);
};

/**
 resolve a type within the active scope
 */
let find_in_scope = ((name, cursor): AST.identifier_t, ctx: t) =>
  switch (Hashtbl.find_opt(ctx.scope.types, Named(name))) {
  | Some(t) => t
  | None =>
    let err = Type.NotFound(name);
    ctx.report(ParseError(TypeError(err), ctx.namespace, cursor));
    Type.K_Invalid(err);
  };
