open Knot.Kore;
open AST;

let to_xml:
  (
    Result.expression_t => Fmt.xml_t(string),
    Type.t => string,
    (list(Result.argument_t), Result.expression_t)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, dump_type, (parameters, result)) =>
    Node(
      "Function",
      [],
      KLambda.Plugin.argument_list_to_xml(expr_to_xml, dump_type, parameters)
      @ [Node("Body", [], [expr_to_xml(result)])],
    );
