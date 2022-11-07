open Knot.Kore;

let parse = Parser.effect;

let pp = Formatter.pp_effect;

let to_xml:
  (
    AST.Expression.expression_t('a) => Fmt.xml_t(string),
    AST.Expression.expression_t('a)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, expr) => Node("Effect", [], [expr_to_xml(expr)]);
