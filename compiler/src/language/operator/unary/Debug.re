open Knot.Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  ((expr_to_xml, _), (operator, expression)) =>
    Fmt.Node(
      AST.Operator.Unary.to_string(operator),
      [],
      [expr_to_xml(expression)],
    );
