open Knot.Kore;
open AST;

let to_xml:
  (
    'typ => string,
    (
      list(KLambda.Interface.Parameter.node_t('expr, 'typ)),
      KExpression.Interface.node_t('typ),
    )
  ) =>
  Fmt.xml_t(string) =
  (dump_type, (parameters, body)) =>
    Node(
      "Function",
      [],
      KLambda.Debug.parameter_list_to_xml(
        KExpression.Plugin.to_xml(dump_type),
        dump_type,
        parameters,
      )
      @ [Node("Body", [], [KExpression.Plugin.to_xml(dump_type, body)])],
    );
