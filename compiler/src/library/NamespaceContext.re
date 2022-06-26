open Infix;
open Reference;

type t = {
  /* unique identifier for this namespace */
  namespace: Namespace.t,
  /* the active module table for the compiler instance */
  modules: ModuleTable.t,
  /* error reporting callback */
  report: Error.compile_err => unit,
  /* error reporting callback */
  mutable inner_modules: list((string, DeclarationTable.t, Range.t)),
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
 find a module by its namespace
 */
let find_module = (namespace: Namespace.t, ctx: t) =>
  ModuleTable.find(namespace, ctx.modules);

/**
 find the type of an export from a different module
 */
let find_export = (namespace: Namespace.t, id: Export.t, ctx: t) => {
  let type_err = Type.ExternalNotFound(namespace, id);

  ctx
  |> find_module(namespace)
  |?< ModuleTable.(get_entry_data % Option.map(({exports}) => exports))
  |> (
    fun
    | Some(exports) =>
      switch (Hashtbl.find_opt(exports, id)) {
      | Some(t) => Ok(t)
      | None => Error(type_err)
      }

    | None => Ok(Invalid(NotInferrable))
  );
};

let define_module =
    (
      module_: string,
      declarations: DeclarationTable.t,
      range: Range.t,
      ctx: t,
    ) =>
  ctx.inner_modules = ctx.inner_modules @ [(module_, declarations, range)];
