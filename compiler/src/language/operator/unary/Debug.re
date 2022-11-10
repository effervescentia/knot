open Knot.Kore;
open AST;

let to_xml = ((expr_to_xml, _), (op, expr)) =>
  Fmt.Node(Operator.Unary.to_string(op), [], [expr_to_xml(expr)]);
