open Infix;
open Reference;

/**
 compilation context used for reporting errors and accessing scope
 */
type t = {
  scope: Scope.t,
  report: Error.compile_err => unit,
};

/* static */

let create = (~scope=Scope.create(), ~report=Error.throw, ()) => {
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
      id: Identifier.t,
      label: option(Identifier.t),
      ctx: t,
    ) => {
  let invalid_type = Type.K_Invalid(ExternalNotFound(namespace, id));
  let type_ =
    switch (ModuleTable.find(namespace, ctx.scope.modules)) {
    | Some({types}) =>
      switch (Hashtbl.find_opt(types, id)) {
      | Some(t) => t
      | None => invalid_type
      }
    | None => invalid_type
    };

  Scope.define(label |?: id, type_, ctx.scope);
};

/**
 resolve a type within the active scope
 */
let find_in_scope = (name: Identifier.t, ctx: t) =>
  switch (Scope.find(name, ctx.scope)) {
  | Type.K_Invalid(err) as t =>
    ctx.report(ParseError(TypeError(err)));
    t;
  | t => t
  };
