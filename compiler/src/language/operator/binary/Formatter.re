open Knot.Kore;

let pp_operator: Fmt.t(AST.Result.binary_t) =
  ppf =>
    (
      fun
      | AST.Result.Equal => KEqual.Plugin.pp
      | AST.Result.Unequal => KUnequal.Plugin.pp

      | AST.Result.LogicalAnd => KLogicalAnd.Plugin.pp
      | AST.Result.LogicalOr => KLogicalOr.Plugin.pp

      | AST.Result.LessOrEqual => KLessOrEqual.Plugin.pp
      | AST.Result.LessThan => KLessThan.Plugin.pp
      | AST.Result.GreaterOrEqual => KGreaterOrEqual.Plugin.pp
      | AST.Result.GreaterThan => KGreaterThan.Plugin.pp

      | AST.Result.Add => KAdd.Plugin.pp
      | AST.Result.Subtract => KSubtract.Plugin.pp
      | AST.Result.Multiply => KMultiply.Plugin.pp
      | AST.Result.Divide => KDivide.Plugin.pp

      | AST.Result.Exponent => KExponentiate.Plugin.pp
    )
    % (pp => pp(ppf, ()));

let pp_binary_operation:
  Fmt.t(AST.Result.raw_expression_t) =>
  Fmt.t(
    (AST.Result.binary_t, AST.Result.expression_t, AST.Result.expression_t),
  ) =
  (pp_expression, ppf, (op, (lhs, _), (rhs, _))) =>
    Fmt.pf(
      ppf,
      "%a %a %a",
      pp_expression,
      lhs,
      pp_operator,
      op,
      pp_expression,
      rhs,
    );
