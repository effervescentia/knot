open Kore;

let rec to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  (dump_type, expr) => {
    let expr_to_xml = to_xml(dump_type);
    let bind = to_xml => to_xml % List.single;
    let unpack =
      Interface.fold(
        ~primitive=bind(KPrimitive.to_xml()),
        ~identifier=bind(KIdentifier.to_xml()),
        ~group=bind(KGroup.to_xml(expr_to_xml)),
        ~closure=bind(KClosure.to_xml((expr_to_xml, dump_type))),
        ~dot_access=bind(KDotAccess.to_xml(expr_to_xml)),
        ~bind_style=bind(KBindStyle.to_xml(expr_to_xml)),
        ~function_call=bind(KFunctionCall.to_xml(expr_to_xml)),
        ~style=bind(KStyle.to_xml(expr_to_xml)),
        ~binary_op=bind(KBinaryOperator.to_xml(expr_to_xml)),
        ~unary_op=bind(KUnaryOperator.to_xml(expr_to_xml)),
        ~ksx=bind(KSX.to_xml((expr_to_xml, dump_type))),
      );

    Dump.node_to_xml(~dump_type, ~unpack, "Expression", expr);
  };
