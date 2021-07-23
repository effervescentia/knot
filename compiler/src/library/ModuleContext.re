open Reference;

type externals_t = Hashtbl.t(Identifier.t, Type2.Result.t);

type t = {
  externals: externals_t,
  definitions: DefinitionTable.t,
  namespace_context: NamespaceContext.t,
};

/* static */

let create =
    (
      ~externals=Hashtbl.create(0),
      ~definitions=DefinitionTable.create(),
      namespace_context: NamespaceContext.t,
    )
    : t => {
  externals,
  definitions,
  namespace_context,
};

/* methods */

/**
 convert the imported externals into a scope
 */
let get_external_scope = (module_context: t) =>
  module_context.externals
  |> Hashtbl.to_seq
  |> Seq.map(Tuple.map_snd2(Type2.Result.to_raw))
  |> NestedHashtbl.from_seq;

/**
 report a compile error
 */
let report = (ctx: t, err: Error.compile_err) =>
  ctx.namespace_context.report(err);

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
  let type_: Type2.Result.t =
    switch (ctx.namespace_context |> NamespaceContext.lookup(namespace, id)) {
    | Ok(t) => `Valid(t)
    | Error(err) =>
      Error.ParseError(
        TypeError(Type2.Result.err_to_strong_err(err)),
        ctx.namespace_context.namespace,
        cursor,
      )
      |> report(ctx);

      `Invalid(err);
    };

  Hashtbl.replace(ctx.externals, label, type_);
};
