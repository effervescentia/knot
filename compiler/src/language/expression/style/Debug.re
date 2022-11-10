open Knot.Kore;
open AST;

let to_xml:
  (
    Expression.expression_t('a) => Fmt.xml_t(string),
    list(Expression.style_rule_t('a))
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
