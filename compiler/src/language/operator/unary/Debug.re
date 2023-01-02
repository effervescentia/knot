open Knot.Kore;
open AST;

let to_xml = ((expr_to_xml, _), (operator, expression)) =>
  Fmt.Node(
    Operator.Unary.to_string(operator),
    [],
    [expr_to_xml(expression)],
  );
