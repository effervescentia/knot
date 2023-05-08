open Kore;

let rec to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  (dump_type, node) =>
    Dump.node_to_xml(
      ~dump_type,
      ~unpack=unpack(dump_type),
      "Expression",
      node,
    )

and unpack = dump_type => {
  let arg = (to_xml(dump_type), dump_type);
  let bind = f => f(arg) % List.single;

  Interface.fold(
    ~primitive=bind(Primitive.to_xml),
    ~identifier=bind(Identifier.to_xml),
    ~group=bind(Group.to_xml),
    ~closure=bind(Closure.to_xml),
    ~dot_access=bind(DotAccess.to_xml),
    ~bind_style=bind(BindStyle.to_xml),
    ~function_call=bind(FunctionCall.to_xml),
    ~style=bind(Style.to_xml),
    ~binary_op=bind(BinaryOperator.to_xml),
    ~unary_op=bind(UnaryOperator.to_xml),
    ~ksx=bind(KSX.to_xml),
  );
};
