open Kore;

let rec pp_expression: Fmt.t(AST.Result.raw_expression_t) =
  ppf =>
    fun
    | Primitive(prim) => prim |> KPrimitive.format(pp_expression, ppf)
    | Identifier(name) => name |> KIdentifier.format(pp_expression, ppf)
    | JSX(jsx) => jsx |> KSX.format(pp_expression, ppf)
    | Group(expr) => expr |> KGroup.format(pp_expression, ppf)
    | BinaryOp(op, lhs, rhs) =>
      (op, lhs, rhs) |> KBinaryOperator.format(pp_expression, ppf)
    | UnaryOp(op, expr) =>
      (op, expr) |> KUnaryOperator.format(pp_expression, ppf)
    | Closure(stmts) => stmts |> KClosure.format(pp_expression, ppf)
    | DotAccess(expr, prop) =>
      (expr, prop) |> KDotAccess.format(pp_expression, ppf)
    | FunctionCall(expr, args) =>
      (expr, args) |> KFunctionCall.format(pp_expression, ppf)
    | Style(rules) => rules |> KStyle.format(pp_expression, ppf);
