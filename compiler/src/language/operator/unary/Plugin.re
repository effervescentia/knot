open Knot.Kore;

let parse = Parser.unary_operation;

let analyze = Analyzer.analyze_unary_operation;

let pp = Formatter.pp_unary_operation;

let to_xml:
  (
    AST.Expression.expression_t('a) => Fmt.xml_t(string),
    (AST.Operator.Unary.t, AST.Expression.expression_t('a))
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, (op, expr)) =>
    Node(AST.Operator.Unary.to_string(op), [], [expr_to_xml(expr)]);
