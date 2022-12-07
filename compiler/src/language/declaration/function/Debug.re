open Knot.Kore;
open AST;

let to_xml:
  (Type.t => string, (list(Result.argument_t), Result.expression_t)) =>
  Fmt.xml_t(string) =
  (dump_type, (parameters, result)) =>
    Node(
      "Function",
      [],
      KLambda.Debug.argument_list_to_xml(
        KExpression.Plugin.to_xml(dump_type),
        dump_type,
        parameters,
      )
      @ [Node("Body", [], [KExpression.Plugin.to_xml(dump_type, result)])],
    );
