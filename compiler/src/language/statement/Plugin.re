open Knot.Kore;

let parse = Parser.statement;

let analyze = Analyzer.analyze_statement;

let pp = Formatter.pp_statement;

let to_xml:
  (
    AST.Expression.expression_t('a) => Fmt.xml_t(string),
    'a => string,
    AST.Expression.statement_t('a)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, dump_type, stmt) =>
    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        (
          fun
          | AST.Expression.Expression(expr) =>
            KEffect.Plugin.to_xml(expr_to_xml, expr)
          | AST.Expression.Variable(name, expr) =>
            KVariable.Plugin.to_xml(expr_to_xml, (name, expr))
        )
        % (x => [x]),
      "Statement",
      stmt,
    );
