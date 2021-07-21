open Reference;

type t = {
  namespace: Namespace.t,
  modules: ModuleTable.t(Type2.t),
  report: Error.compile_err => unit,
};

/* static */

let create =
    (
      ~modules=ModuleTable.create(0),
      ~report=Error.throw,
      namespace: Namespace.t,
    )
    : t => {
  namespace,
  modules,
  report,
};
