open Kore;

let to_xml:
  (
    Node.t('expr, 'typ) => Fmt.xml_t(string),
    'typ => string,
    AST.Statement.node_t('expr, 'typ)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, dump_type, statement) => {
    let bind = to_xml => to_xml(expr_to_xml) % List.single;
    let unpack =
      Util.fold(
        ~variable=bind(KVariable.to_xml),
        ~effect=bind(KEffect.to_xml),
      );

    Dump.node_to_xml(~dump_type, ~unpack, "Statement", statement);
  };
