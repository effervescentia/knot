open Knot.Kore;
open AST;

let to_xml:
  (
    Expression.expression_t('a) => Fmt.xml_t(string),
    'a => string,
    Expression.statement_t('a)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, dump_type, stmt) =>
    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        AST.Expression.(
          fun
          | Expression(expr) => KEffect.Plugin.to_xml(expr_to_xml, expr)
          | Variable(name, expr) =>
            KVariable.Plugin.to_xml(expr_to_xml, (name, expr))
        )
        % (x => [x]),
      "Statement",
      stmt,
    );
