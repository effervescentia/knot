open Reference;

type t = {
  /* unique identifier for this namespace */
  namespace: Namespace.t,
  /* the active module table for the compiler instance */
  modules: ModuleTable.t,
  /* error reporting callback */
  report: Error.compile_err => unit,
  /* error reporting callback */
  mutable inner_modules: list((Module.t, DefinitionTable.t, Cursor.t)),
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
  inner_modules: [],
};

/* methods */

/**
 find the type of an export from a different module
 */
let lookup = (namespace: Namespace.t, id: Export.t, ctx: t) => {
  let type_err = Type.Error.ExternalNotFound(namespace, id);

  switch (ModuleTable.find(namespace, ctx.modules)) {
  | Some({exports}) =>
    switch (Hashtbl.find_opt(exports, id)) {
    | Some(Valid(t)) => Ok(t)
    | Some(Invalid(type_err)) => Error(type_err)
    | None => Error(type_err)
    }
  | None => Error(type_err)
  };
};

let define_module =
    (
      module_: Module.t,
      definitions: DefinitionTable.t,
      cursor: Cursor.t,
      ctx: t,
    ) =>
  ctx.inner_modules = ctx.inner_modules @ [(module_, definitions, cursor)];
