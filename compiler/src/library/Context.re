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
