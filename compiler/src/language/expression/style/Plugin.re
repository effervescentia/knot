open Knot.Kore;

let parse = Parser.style_expression;

let analyze = Analyzer.analyze_style;

let pp = Formatter.pp_style_expression;

let to_xml:
  (
    AST.Expression.expression_t('a) => Fmt.xml_t(string),
    list(AST.Expression.style_rule_t('a))
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, rules) =>
    Node(
      "Style",
      [],
      rules
      |> List.map(rule =>
           Dump.node_to_xml(
             ~unpack=
               ((prop, expr)) =>
                 [
                   Dump.node_to_xml(~dump_value=Fun.id, "RuleProperty", prop),
                   Node("RuleExpression", [], [expr_to_xml(expr)]),
                 ],
             "Rule",
             rule,
           )
         ),
    );
