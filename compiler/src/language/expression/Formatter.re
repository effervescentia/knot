open Kore;

let rec format: Fmt.t(AST.Result.raw_expression_t) =
  ppf => {
    let (&>) = (args, formatter) => args |> formatter(format, ppf);

    fun
    | Primitive(prim) => prim &> KPrimitive.format
    | Identifier(name) => name &> KIdentifier.format
    | KSX(ksx) => ksx &> KSX.format
    | Group(expr) => expr &> KGroup.format
    | BinaryOp(op, lhs, rhs) => (op, lhs, rhs) &> KBinaryOperator.format
    | UnaryOp(op, expr) => (op, expr) &> KUnaryOperator.format
    | Closure(stmts) => stmts &> KClosure.format
    | DotAccess(expr, prop) => (expr, prop) &> KDotAccess.format
    | BindStyle(kind, view, style) =>
      (kind, view, style) &> KBindStyle.format
    | FunctionCall(expr, args) => (expr, args) &> KFunctionCall.format
    | Style(rules) => rules &> KStyle.format;
  };
