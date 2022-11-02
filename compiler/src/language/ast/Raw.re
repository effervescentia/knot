include Factory.Make({
  type type_t = unit;

  let node_to_entity = (~attributes=[], ~children=[], label, node) =>
    Common.Dump.node_to_entity(
      (_, ()) => (),
      ~attributes,
      ~children,
      label,
      node,
    );
});
