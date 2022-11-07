open Knot.Kore;

let parse = Parser.closure;

let analyze = Analyzer.analyze_closure;

let pp = Formatter.pp_closure;

let to_xml:
  (
    AST.Expression.statement_t('a) => Fmt.xml_t(string),
    list(AST.Expression.statement_t('a))
  ) =>
  Fmt.xml_t(string) =
  (stmt_to_xml, stmts) =>
    Node("Closure", [], stmts |> List.map(stmt_to_xml));
