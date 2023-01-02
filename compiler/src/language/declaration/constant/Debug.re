open Knot.Kore;

let to_xml:
  (AST.Type.t => string, AST.Result.expression_t) => Fmt.xml_t(string) =
  (type_to_string, expression) =>
    Node(
      "Constant",
      [],
      [KExpression.Plugin.to_xml(type_to_string, expression)],
    );
