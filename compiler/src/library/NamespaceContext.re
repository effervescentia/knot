open Reference;

type t = {
  namespace: Namespace.t,
  modules: ModuleTable.t(Type2.Result.t),
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

/* methods */

/**
 find the type of an export from a different module
 */
let lookup = (namespace: Namespace.t, id: Export.t, ctx: t) => {
  let type_err = Type2.ExternalNotFound(namespace, id);

  switch (ModuleTable.find(namespace, ctx.modules)) {
  | Some({types}) =>
    switch (Hashtbl.find_opt(types, id)) {
    | Some(`Invalid(type_err)) => Error(type_err)
    | Some(t) => Ok(t)
    | None => Error(type_err)
    }
  | None => Error(type_err)
  };
};
