open Knot.Kore;

let parse = Parser.group;

let analyze = Analyzer.analyze_group;

let pp = Formatter.pp_group;

let to_xml:
  (
    AST.Expression.expression_t('a) => Fmt.xml_t(string),
    AST.Expression.expression_t('a)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, expr) => Node("Group", [], [expr_to_xml(expr)]);
