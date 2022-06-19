open Reference;

type t = {
  /* types that have been imported into the scope */
  externals: Scope.type_lookup_t,
  /* types that have been declared within the scope */
  declarations: Hashtbl.t(Export.t, Type.t),
  /* parent namespace context */
  namespace_context: TypingNamespaceContext.t,
};
