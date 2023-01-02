open Kore;

let rec format: Fmt.t(AST.Result.raw_expression_t) =
  ppf =>
    fun
    | Primitive(prim) => prim |> KPrimitive.format(format, ppf)
    | Identifier(name) => name |> KIdentifier.format(format, ppf)
    | KSX(ksx) => ksx |> KSX.format(format, ppf)
    | Group(expr) => expr |> KGroup.format(format, ppf)
    | BinaryOp(op, lhs, rhs) =>
      (op, lhs, rhs) |> KBinaryOperator.format(format, ppf)
    | UnaryOp(op, expr) => (op, expr) |> KUnaryOperator.format(format, ppf)
    | Closure(stmts) => stmts |> KClosure.format(format, ppf)
    | DotAccess(expr, prop) =>
      (expr, prop) |> KDotAccess.format(format, ppf)
    | BindStyle(source, view, style) =>
      (source, view, style) |> KBindStyle.format(format, ppf)
    | FunctionCall(expr, args) =>
      (expr, args) |> KFunctionCall.format(format, ppf)
    | Style(rules) => rules |> KStyle.format(format, ppf);
