open Knot.Kore;
open AST;

let to_xml = (dump_type, (parameters, mixins, result)) =>
  Fmt.Node(
    "View",
    [],
    KLambda.Debug.argument_list_to_xml(
      KExpression.Plugin.to_xml(dump_type),
      dump_type,
      parameters,
    )
    @ [Node("Body", [], [KExpression.Plugin.to_xml(dump_type, result)])],
  );
