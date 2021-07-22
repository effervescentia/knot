open Reference;

type externals_t = Hashtbl.t(Identifier.t, Type2.t);

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

let get_external_scope = (module_context: t) =>
  module_context.externals
  |> Hashtbl.to_seq
  |> Seq.map(Tuple.map_snd2(Type2.to_raw))
  |> NestedHashtbl.from_seq;

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
  let type_ =
    switch (ctx.namespace_context |> NamespaceContext.lookup(namespace, id)) {
    | Ok(t) => t
    | Error(err) =>
      Error.ParseError(
        TypeError(err),
        ctx.namespace_context.namespace,
        cursor,
      )
      |> report(ctx);

      Type.K_Invalid(err);
    };

  Scope.define(label, type_, ctx.scope);
};
