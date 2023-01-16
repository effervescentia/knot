open Knot.Kore;

let to_xml:
  ('typ => string, KExpression.Interface.node_t('typ)) => Fmt.xml_t(string) =
  (type_to_string, expression) =>
    Node(
      "Constant",
      [],
      [KExpression.Plugin.to_xml(type_to_string, expression)],
    );
