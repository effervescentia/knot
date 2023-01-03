open Knot.Kore;

let to_xml: ('expr => Fmt.xml_t(string), 'expr) => Fmt.xml_t(string) =
  (expr_to_xml, expression) =>
    Node("Effect", [], [expr_to_xml(expression)]);
