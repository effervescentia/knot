open Knot.Kore;

let parse = Parser.function_;

let pp = Formatter.pp_function;

let to_xml:
  (
    AST.Result.expression_t => Fmt.xml_t(string),
    AST.Type.t => string,
    (list(AST.Result.argument_t), AST.Result.expression_t)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, dump_type, (parameters, result)) =>
    Node(
      "Function",
      [],
      KLambda.Plugin.argument_list_to_xml(expr_to_xml, dump_type, parameters)
      @ [Node("Body", [], [expr_to_xml(result)])],
    );
