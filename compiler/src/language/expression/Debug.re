open Knot.Kore;
open AST;

let rec to_xml:
  ('a => string, Expression.expression_t('a)) => Fmt.xml_t(string) =
  (dump_type, expr) => {
    let expr_to_xml = to_xml(dump_type);
    let stmt_to_xml = KStatement.Plugin.to_xml(expr_to_xml, dump_type);

    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        Expression.(
          fun
          | Primitive(prim) => KPrimitive.Plugin.to_xml(prim)
          | Identifier(id) => KIdentifier.Plugin.to_xml(id)
          | Group(expr) => KGroup.Plugin.to_xml(expr_to_xml, expr)
          | Closure(stmts) => KClosure.Plugin.to_xml(stmt_to_xml, stmts)
          | DotAccess(root, prop) =>
            KDotAccess.Plugin.to_xml(expr_to_xml, (root, prop))
          | FunctionCall(name, args) =>
            KFunctionCall.Plugin.to_xml(expr_to_xml, (name, args))
          | Style(rules) => KStyle.Plugin.to_xml(expr_to_xml, rules)
          | BinaryOp(op, lhs, rhs) =>
            KBinaryOperator.Plugin.to_xml(expr_to_xml, (op, lhs, rhs))
          | UnaryOp(op, expr) =>
            KUnaryOperator.Plugin.to_xml(expr_to_xml, (op, expr))
          | JSX(jsx) => KSX.Plugin.to_xml(expr_to_xml, dump_type, jsx)
        )
        % (x => [x]),
      "Expression",
      expr,
    );
  };
