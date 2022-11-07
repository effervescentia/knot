open Knot.Kore;

let parse = Parser.dot_access;

let analyze = Analyzer.analyze_dot_access;

let pp = Formatter.pp_dot_access;

let to_xml:
  (
    AST.Expression.expression_t('a) => Fmt.xml_t(string),
    (AST.Expression.expression_t('a), AST.Common.untyped_t(string))
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, (root, property)) =>
    Node(
      "DotAccess",
      [],
      [
        Node("Object", [], [expr_to_xml(root)]),
        Dump.node_to_xml(~dump_value=Fun.id, "Property", property),
      ],
    );
