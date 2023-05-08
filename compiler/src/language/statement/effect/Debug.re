open Knot.Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  (expr_to_xml, expression) =>
    Node("Effect", [], [expr_to_xml(expression)]);
