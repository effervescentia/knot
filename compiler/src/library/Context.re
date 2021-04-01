/**
 compilation context used for reporting errors and accessing scope
 */
type t = {
  scope: Scope.t,
  report: Error.compile_err => unit,
};
