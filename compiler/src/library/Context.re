open Infix;
open Reference;

/**
 compilation context used for reporting errors and accessing scope
 */
type t = {
  parent: option(t),
  namespace: Namespace.t,
  scope: Scope.t,
  report: Error.compile_err => unit,
  mutable children: list((t, Cursor.t)),
};

/* static */

let create =
    (~scope=Scope.create(), ~report=Error.throw, namespace: Namespace.t) => {
  parent: None,
  namespace,
  scope,
  report,
  children: [],
};

/* methods */

/**
 clone the current context and set the original context as its parent
 */
let child = (ctx: t) => {
  ...ctx,
  scope: ctx.scope |> Scope.clone,
  parent: Some(ctx),
};

/**
 save the context to its parent with an associated cursor
 */
let submit = (cursor: Cursor.t, ctx: t) =>
  switch (ctx.parent) {
  | Some(parent) => parent.children = parent.children @ [(ctx, cursor)]
  | None => ()
  };

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
      /* ctx.report(ParseError(TypeError(err), ctx.namespace, cursor)); */
      Type.K_Invalid(err)
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
    /* ctx.report(ParseError(TypeError(err), ctx.namespace, cursor)); */
    Type.K_Invalid(err);
  };
