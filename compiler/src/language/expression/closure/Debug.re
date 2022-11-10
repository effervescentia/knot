open Knot.Kore;
open AST;

let to_xml:
  (
    Expression.statement_t('a) => Fmt.xml_t(string),
    list(Expression.statement_t('a))
  ) =>
  Fmt.xml_t(string) =
  (stmt_to_xml, stmts) =>
    Node("Closure", [], stmts |> List.map(stmt_to_xml));
