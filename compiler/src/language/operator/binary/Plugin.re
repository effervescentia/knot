open Knot.Kore;
open AST;

let analyze = Analyzer.analyze_binary_operation;

let pp = Formatter.pp_binary_operation;

let to_xml:
  (
    Expression.expression_t('a) => Fmt.xml_t(string),
    (
      Operator.Binary.t,
      Expression.expression_t('a),
      Expression.expression_t('a),
    )
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, (op, lhs, rhs)) =>
    Node(
      Operator.Binary.to_string(op),
      [],
      [
        Node("Left", [], [expr_to_xml(lhs)]),
        Node("Right", [], [expr_to_xml(rhs)]),
      ],
    );
