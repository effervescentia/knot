open Reference;

type t = {
  /* unique identifier for this namespace */
  namespace: Namespace.t,
  /* error reporting callback */
  report: Error.compile_err => unit,
  /* module exports */
  mutable modules: list((Module.t, DefinitionTable.t)),
};

/* static */

let create = (~report=Error.throw, namespace: Namespace.t) => {
  namespace,
  report,
  modules: [],
};

/* methods */

let generate_types = (ctx: t) =>
  ctx.modules
  |> List.map(((module_, exports)) =>
       (module_, DefinitionTable.to_module_type(exports))
     );
