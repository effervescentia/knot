open Knot.Kore;

let parse = Parser.constant;

let pp = Formatter.pp_constant;

let to_xml:
  (AST.Type.t => string, AST.Result.expression_t) => Fmt.xml_t(string) =
  (type_to_string, expr) =>
    Node("Constant", [], [KExpression.Plugin.to_xml(type_to_string, expr)]);
