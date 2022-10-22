open Knot.Kore;

let parse = Parser.expression;

let rec pp: Fmt.t(AST.raw_expression_t) =
  ppf =>
    fun
    | Primitive(prim) => prim |> KPrimitive.Plugin.pp(ppf)
    | Identifier(name) => name |> KIdentifier.Plugin.pp(ppf)
    | JSX(jsx) => jsx |> KSX.Plugin.pp(pp, ppf)
    | Group(expr) => expr |> KGroup.Plugin.pp(pp, ppf)
    | BinaryOp(op, lhs, rhs) =>
      (op, lhs, rhs) |> KBinaryOperator.Plugin.pp(pp, ppf)
    | UnaryOp(op, expr) => (op, expr) |> KUnaryOperator.Plugin.pp(pp, ppf)
    | Closure(stmts) =>
      stmts |> KClosure.Plugin.pp(KStatement.Plugin.pp(pp), ppf)
    | DotAccess(expr, prop) => (expr, prop) |> KDotAccess.Plugin.pp(pp, ppf)
    | FunctionCall(expr, args) =>
      (expr, args) |> KFunctionCall.Plugin.pp(pp, ppf)
    | Style(rules) => KStyle.Plugin.pp(pp, ppf, rules);
