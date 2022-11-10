open Kore;

let rec pp_expression: Fmt.t(AST.Result.raw_expression_t) =
  ppf =>
    fun
    | Primitive(prim) => prim |> KPrimitive.pp((), ppf)
    | Identifier(name) => name |> KIdentifier.pp((), ppf)
    | JSX(jsx) => jsx |> KSX.pp(pp_expression, ppf)
    | Group(expr) => expr |> KGroup.pp(pp_expression, ppf)
    | BinaryOp(op, lhs, rhs) =>
      (op, lhs, rhs) |> KBinaryOperator.pp(pp_expression, ppf)
    | UnaryOp(op, expr) =>
      (op, expr) |> KUnaryOperator.pp(pp_expression, ppf)
    | Closure(stmts) =>
      stmts |> KClosure.pp(KStatement.Plugin.pp(pp_expression), ppf)
    | DotAccess(expr, prop) =>
      (expr, prop) |> KDotAccess.pp(pp_expression, ppf)
    | FunctionCall(expr, args) =>
      (expr, args) |> KFunctionCall.pp(pp_expression, ppf)
    | Style(rules) => rules |> KStyle.pp(pp_expression, ppf);
