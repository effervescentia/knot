open Knot.Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  ((expr_to_xml, _), rules) =>
    Fmt.Node(
      "Style",
      [],
      rules
      |> List.map(rule =>
           Dump.node_to_xml(
             ~unpack=
               ((prop, expr)) =>
                 [
                   Dump.identifier_to_xml("RuleProperty", prop),
                   Node("RuleExpression", [], [expr_to_xml(expr)]),
                 ],
             "Rule",
             rule,
           )
         ),
    );
