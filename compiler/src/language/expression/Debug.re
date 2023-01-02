open Kore;
open AST;

let rec to_xml:
  ('a => string, Expression.expression_t('a)) => Fmt.xml_t(string) =
  (dump_type, expr) => {
    let expr_to_xml = to_xml(dump_type);
    let arg = (expr_to_xml, dump_type);

    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        Expression.(
          fun
          | Primitive(prim) => prim |> KPrimitive.to_xml(arg)
          | Identifier(id) => id |> KIdentifier.to_xml(arg)
          | Group(expr) => expr |> KGroup.to_xml(arg)
          | Closure(stmts) => stmts |> KClosure.to_xml(arg)
          | DotAccess(root, prop) => (root, prop) |> KDotAccess.to_xml(arg)
          | BindStyle(kind, view, style) =>
            (kind, view, style) |> KBindStyle.to_xml(arg)
          | FunctionCall(name, args) =>
            (name, args) |> KFunctionCall.to_xml(arg)
          | Style(rules) => rules |> KStyle.to_xml(arg)
          | BinaryOp(op, lhs, rhs) =>
            (op, lhs, rhs) |> KBinaryOperator.to_xml(arg)
          | UnaryOp(op, expr) => (op, expr) |> KUnaryOperator.to_xml(arg)
          | KSX(ksx) => KSX.to_xml(arg, ksx)
        )
        % (x => [x]),
      "Expression",
      expr,
    );
  };
