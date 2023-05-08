open Kore;
open AST;

let to_xml:
  (
    'typ => string,
    (list(Lambda.Parameter.node_t('expr, 'typ)), Expression.node_t('typ))
  ) =>
  Fmt.xml_t(string) =
  (dump_type, (parameters, body)) =>
    Node(
      "Function",
      [],
      Lambda.parameter_list_to_xml(
        Expression.to_xml(dump_type),
        dump_type,
        parameters,
      )
      @ [Node("Body", [], [Expression.to_xml(dump_type, body)])],
    );
