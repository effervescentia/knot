open Knot.Kore;

let parse = Parser.expression;
let parse_jsx_term = Parser.jsx_term;

let analyze = Analyzer.analyze_expression;

let pp = Formatter.pp_expression;

let rec to_xml:
  ('a => string, AST.Expression.expression_t('a)) => Fmt.xml_t(string) =
  (dump_type, expr) => {
    let expr_to_xml = to_xml(dump_type);
    let stmt_to_xml = KStatement.Plugin.to_xml(expr_to_xml, dump_type);

    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        (
          fun
          | AST.Expression.Primitive(prim) => KPrimitive.Plugin.to_xml(prim)
          | AST.Expression.Identifier(id) => KIdentifier.Plugin.to_xml(id)
          | AST.Expression.Group(expr) =>
            KGroup.Plugin.to_xml(expr_to_xml, expr)
          | AST.Expression.Closure(stmts) =>
            KClosure.Plugin.to_xml(stmt_to_xml, stmts)
          | AST.Expression.DotAccess(root, prop) =>
            KDotAccess.Plugin.to_xml(expr_to_xml, (root, prop))
          | AST.Expression.FunctionCall(name, args) =>
            KFunctionCall.Plugin.to_xml(expr_to_xml, (name, args))
          | AST.Expression.Style(rules) =>
            KStyle.Plugin.to_xml(expr_to_xml, rules)
          | AST.Expression.BinaryOp(op, lhs, rhs) =>
            KBinaryOperator.Plugin.to_xml(expr_to_xml, (op, lhs, rhs))
          | AST.Expression.UnaryOp(op, expr) =>
            KUnaryOperator.Plugin.to_xml(expr_to_xml, (op, expr))
          | AST.Expression.JSX(jsx) =>
            KSX.Plugin.to_xml(expr_to_xml, dump_type, jsx)
        )
        % (x => [x]),
      "Expression",
      expr,
    );
  };
