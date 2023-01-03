open Kore;

let rec to_xml:
  ('a => string, AST.Expression.node_t('a)) => Fmt.xml_t(string) =
  (dump_type, expr) => {
    let expr_to_xml = to_xml(dump_type);
    let bind = to_xml => to_xml((expr_to_xml, dump_type)) % List.single;
    let unpack =
      Util.fold(
        ~primitive=bind(KPrimitive.to_xml),
        ~identifier=bind(KIdentifier.to_xml),
        ~group=bind(KGroup.to_xml),
        ~closure=bind(KClosure.to_xml),
        ~dot_access=bind(KDotAccess.to_xml),
        ~bind_style=bind(KBindStyle.to_xml),
        ~function_call=bind(KFunctionCall.to_xml),
        ~style=bind(KStyle.to_xml),
        ~binary_op=bind(KBinaryOperator.to_xml),
        ~unary_op=bind(KUnaryOperator.to_xml),
        ~ksx=bind(KSX.to_xml),
      );

    Dump.node_to_xml(~dump_type, ~unpack, "Expression", expr);
  };
