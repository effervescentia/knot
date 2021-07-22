open Reference;

type t = {
  parent: option(t),
  namespace_context: NamespaceContext.t,
  scope: NestedHashtbl.t(Identifier.t, Type2.Raw.t),
};

/* static */

let create =
    (
      ~parent: option(t)=?,
      ~scope=NestedHashtbl.create(0),
      namespace_context: NamespaceContext.t,
    )
    : t => {
  namespace_context,
  parent,
  scope,
};

let from_module = (module_context: ModuleContext.t): t =>
  create(
    ~scope=ModuleContext.get_external_scope(module_context),
    module_context.namespace_context,
  );

/* methods */

let child = (parent: t) =>
  create(
    ~parent,
    ~scope=NestedHashtbl.child(parent.scope),
    parent.namespace_context,
  );

let report = (ctx: t, err: Error.compile_err) =>
  ctx.namespace_context.report(err);
