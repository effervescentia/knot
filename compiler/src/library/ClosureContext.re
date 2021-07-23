open Reference;

type t = {
  parent: option(t),
  namespace_context: NamespaceContext.t,
  scope: NestedHashtbl.t(Identifier.t, Type2.Raw.t),
  mutable children: list((t, Cursor.t)),
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
  children: [],
};

let from_module = (module_context: ModuleContext.t): t =>
  create(
    ~scope=ModuleContext.get_external_scope(module_context),
    module_context.namespace_context,
  );

/* methods */

/**
 create a context which is a child of the one provided
 */
let child = (parent: t) =>
  create(
    ~parent,
    ~scope=NestedHashtbl.child(parent.scope),
    parent.namespace_context,
  );

/**
 report a compile error
 */
let report = (ctx: t, err: Error.compile_err) =>
  ctx.namespace_context.report(err);

/**
 resolve a type within the active scope
 */
let resolve = ((name, cursor): AST.identifier_t, ctx: t) =>
  switch (ctx.scope |> NestedHashtbl.find(name)) {
  | Some(t) => t
  | None =>
    let err = Type2.Error.NotFound(name);
    /* ctx.report(ParseError(TypeError(err), ctx.namespace, cursor)); */
    `Invalid(err);
  };

/**
 define a new variable within the scope
 */
let define = (name: Identifier.t, type_: Type2.Raw.t, ctx: t) =>
  ctx.scope |> NestedHashtbl.set(name, type_);

/**
 define a new weak variable within the scope
 */
let define_weak = (id: Identifier.t, ctx: t): Type2.Raw.t => {
  let type_ = `Weak(ref(Ok(`Abstract(Type2.Trait.Unknown))));

  ctx.scope |> NestedHashtbl.set(id, type_);

  type_;
};

/**
 save a context to its parent with an associated cursor
 */
let save = (cursor: Cursor.t, ctx: t) =>
  switch (ctx.parent) {
  | Some(parent) => parent.children = parent.children @ [(ctx, cursor)]
  | None => ()
  };
