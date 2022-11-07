open Knot.Kore;

let parse = Parser.variable;

let pp = Formatter.pp_variable;

let to_xml:
  (
    AST.Expression.expression_t('a) => Fmt.xml_t(string),
    (AST.Common.untyped_t(string), AST.Expression.expression_t('a))
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, (name, expr)) =>
    Node(
      "Variable",
      [],
      [
        Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
        Node("Value", [], [expr_to_xml(expr)]),
      ],
    );
