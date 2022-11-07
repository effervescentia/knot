open Knot.Kore;

let analyze = Analyzer.analyze_binary_operation;

let pp = Formatter.pp_binary_operation;

let to_xml:
  (
    AST.Expression.expression_t('a) => Fmt.xml_t(string),
    (
      AST.Operator.Binary.t,
      AST.Expression.expression_t('a),
      AST.Expression.expression_t('a),
    )
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, (op, lhs, rhs)) =>
    Node(
      AST.Operator.Binary.to_string(op),
      [],
      [
        Node("Left", [], [expr_to_xml(lhs)]),
        Node("Right", [], [expr_to_xml(rhs)]),
      ],
    );
