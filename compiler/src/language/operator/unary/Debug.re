open Knot.Kore;
open AST;

let to_xml:
  (
    Expression.expression_t('a) => Fmt.xml_t(string),
    (Operator.Unary.t, Expression.expression_t('a))
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, (op, expr)) =>
    Node(Operator.Unary.to_string(op), [], [expr_to_xml(expr)]);
