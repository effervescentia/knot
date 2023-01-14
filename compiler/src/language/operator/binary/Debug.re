open Knot.Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  (expr_to_xml, (operator, lhs, rhs)) =>
    Fmt.Node(
      AST.Operator.Binary.to_string(operator),
      [],
      [
        Node("Left", [], [expr_to_xml(lhs)]),
        Node("Right", [], [expr_to_xml(rhs)]),
      ],
    );
