open Knot.Kore;

let rec pp_expression: Fmt.t(AST.Result.raw_expression_t) =
  ppf =>
    fun
    | Primitive(prim) => prim |> KPrimitive.Plugin.pp(ppf)
    | Identifier(name) => name |> KIdentifier.Plugin.pp(ppf)
    | JSX(jsx) => jsx |> KSX.Plugin.pp(pp_expression, ppf)
    | Group(expr) => expr |> KGroup.Plugin.pp(pp_expression, ppf)
    | BinaryOp(op, lhs, rhs) =>
      (op, lhs, rhs) |> KBinaryOperator.Plugin.pp(pp_expression, ppf)
    | UnaryOp(op, expr) =>
      (op, expr) |> KUnaryOperator.Plugin.pp(pp_expression, ppf)
    | Closure(stmts) =>
      stmts |> KClosure.Plugin.pp(KStatement.Plugin.pp(pp_expression), ppf)
    | DotAccess(expr, prop) =>
      (expr, prop) |> KDotAccess.Plugin.pp(pp_expression, ppf)
    | FunctionCall(expr, args) =>
      (expr, args) |> KFunctionCall.Plugin.pp(pp_expression, ppf)
    | Style(rules) => KStyle.Plugin.pp(pp_expression, ppf, rules);
