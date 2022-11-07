open Knot.Kore;

let parse = Parser.function_call;

let analyze = Analyzer.analyze_function_call;

let pp = Formatter.pp_function_call;

let to_xml:
  (
    AST.Expression.expression_t('a) => Fmt.xml_t(string),
    (
      AST.Expression.expression_t('a),
      list(AST.Expression.expression_t('a)),
    )
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
