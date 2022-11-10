open Knot.Kore;
open AST;

let to_xml:
  (
    Expression.expression_t('a) => Fmt.xml_t(string),
    (Expression.expression_t('a), list(Expression.expression_t('a)))
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, (name, args)) =>
    Node(
      "FunctionCall",
      [],
      [
        Node("Name", [], [expr_to_xml(name)]),
        Node("Arguments", [], args |> List.map(expr_to_xml)),
      ],
    );
