open Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  ((expr_to_xml, dump_type), statement) => {
    let bind = to_xml => to_xml(expr_to_xml) % List.single;
    let unpack =
      Interface.fold(
        ~variable=bind(KVariable.to_xml),
        ~effect=bind(KEffect.to_xml),
      );

    Dump.node_to_xml(~dump_type, ~unpack, "Statement", statement);
  };
