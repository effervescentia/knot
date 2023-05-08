open Kore;

let unpack = ((expr_to_xml, dump_type)) => {
  let bind = to_xml => to_xml(expr_to_xml) % List.single;

  Interface.fold(
    ~variable=bind(Variable.to_xml),
    ~effect=bind(Effect.to_xml),
  );
};

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  ((expr_to_xml, dump_type) as arg, statement) => {
    Dump.node_to_xml(
      ~dump_type,
      ~unpack=unpack(arg),
      "Statement",
      statement,
    );
  };
