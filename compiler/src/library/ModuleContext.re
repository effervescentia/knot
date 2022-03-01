open Reference;

type t = {
  /* types that have been imported into the scope */
  externals: Scope.type_lookup_t,
  /* types that have been declared within the scope */
  declarations: DeclarationTable.t,
  /* parent namespace context */
  namespace_context: NamespaceContext.t,
};

/* static */

let create =
    (
      ~externals=Hashtbl.create(0),
      ~declarations=DeclarationTable.create(),
      namespace_context: NamespaceContext.t,
    )
    : t => {
  externals,
  declarations,
  namespace_context,
};

/* methods */

/* compile all externals and declarations into a scope */
let to_scope = (range: Range.t, ctx: t): Scope.t => {
  let types =
    DeclarationTable.to_lookup_seq(ctx.declarations)
    |> Seq.append(Hashtbl.to_seq(ctx.externals))
    |> Hashtbl.of_seq;

  {
    ...
      Scope.create(
        ctx.namespace_context.namespace,
        ctx.namespace_context.report,
        range,
      ),
    types,
  };
};

/**
 report a compile error
 */
let report = (ctx: t, err: Error.compile_err) =>
  ctx.namespace_context.report(err);

/**
 add a new declaration to the module
 */
let declare = (~main=false, name: Identifier.t, type_: Type.t, ctx: t) => {
  ctx.declarations |> DeclarationTable.add(Export.Named(name), type_);

  if (main) {
    ctx.declarations |> DeclarationTable.add(Export.Main, type_);
  };
};

/**
 find the type of an export from a different module and import it into the current scope
 */
let import =
    (
      namespace: Namespace.t,
      (id, range): (Export.t, Range.t),
      label: Identifier.t,
      ctx: t,
    ) => {
  let type_: Type.t =
    switch (ctx.namespace_context |> NamespaceContext.lookup(namespace, id)) {
    | Ok(t) => t
    | Error(err) =>
      Error.ParseError(
        TypeError(err),
        ctx.namespace_context.namespace,
        range,
      )
      |> report(ctx);

      Invalid(NotInferrable);
    };

  Hashtbl.replace(ctx.externals, label, type_);
};
