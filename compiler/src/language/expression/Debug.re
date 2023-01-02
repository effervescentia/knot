open Kore;
open AST;

let rec to_xml:
  ('a => string, Expression.expression_t('a)) => Fmt.xml_t(string) =
  (dump_type, expr) => {
    let expr_to_xml = to_xml(dump_type);
    let (&>) = (args, to_xml) =>
      args |> to_xml((expr_to_xml, dump_type)) |> List.single;

    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        Expression.(
          fun
          | Primitive(primitive) => primitive &> KPrimitive.to_xml
          | Identifier(name) => name &> KIdentifier.to_xml
          | Group(expression) => expression &> KGroup.to_xml
          | Closure(statements) => statements &> KClosure.to_xml
          | DotAccess(object_, property) =>
            (object_, property) &> KDotAccess.to_xml
          | BindStyle(kind, view, style) =>
            (kind, view, style) &> KBindStyle.to_xml
          | FunctionCall(function_, arguments) =>
            (function_, arguments) &> KFunctionCall.to_xml
          | Style(rules) => rules &> KStyle.to_xml
          | BinaryOp(operator, lhs, rhs) =>
            (operator, lhs, rhs) &> KBinaryOperator.to_xml
          | UnaryOp(operator, expression) =>
            (operator, expression) &> KUnaryOperator.to_xml
          | KSX(ksx) => ksx &> KSX.to_xml
        ),
      "Expression",
      expr,
    );
  };
