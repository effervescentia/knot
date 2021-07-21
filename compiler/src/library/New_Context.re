open Infix;
open Reference;

/**
 compilation context used for reporting errors and accessing scope
 */
type t = {
  namespace: Namespace.t,
  report: Error.compile_err => unit,
  scope: New_Scope.t,
  raw_scope: New_RawScope.t,
};

/* static */

let create =
    (
      ~scope=New_Scope.create(),
      ~raw_scope=New_RawScope.create(),
      ~report=Error.throw,
      namespace: Namespace.t,
    ) => {
  namespace,
  scope,
  raw_scope,
  report,
};

/* methods */

let attach = (cursor: Cursor.t, ctx: t): t => {
  ...ctx,
  scope: New_Scope.child(ctx.scope, cursor),
  raw_scope: New_RawScope.child(ctx.raw_scope),
};
