open Knot.Kore;
open AST;

let to_xml = ((expr_to_xml, _), (operator, lhs, rhs)) =>
  Fmt.Node(
    Operator.Binary.to_string(operator),
    [],
    [
      Node("Left", [], [expr_to_xml(lhs)]),
      Node("Right", [], [expr_to_xml(rhs)]),
    ],
  );
