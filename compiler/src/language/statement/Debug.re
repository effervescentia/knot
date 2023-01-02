open Kore;
open AST;

let to_xml:
  (
    Expression.expression_t('a) => Fmt.xml_t(string),
    'a => string,
    Expression.statement_t('a)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, dump_type, stmt) => {
    let (&>) = (args, to_xml) => args |> to_xml(expr_to_xml) |> List.single;

    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        AST.Expression.(
          fun
          | Effect(expression) => expression &> KEffect.to_xml
          | Variable(name, expression) =>
            (name, expression) &> KVariable.to_xml
        ),
      "Statement",
      stmt,
    );
  };
