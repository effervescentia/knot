open Kore;
open AST;

let to_xml = (dump_type, (parameters, mixins, result)) =>
  Fmt.Node(
    "View",
    [],
    Lambda.parameter_list_to_xml(
      Expression.to_xml(dump_type),
      dump_type,
      parameters,
    )
    @ [Node("Body", [], [Expression.to_xml(dump_type, result)])],
  );
