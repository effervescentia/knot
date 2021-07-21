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
