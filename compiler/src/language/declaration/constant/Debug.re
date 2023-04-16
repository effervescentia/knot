open Kore;

let to_xml: ('typ => string, Expression.node_t('typ)) => Fmt.xml_t(string) =
  (type_to_string, expression) =>
    Node("Constant", [], [Expression.to_xml(type_to_string, expression)]);
