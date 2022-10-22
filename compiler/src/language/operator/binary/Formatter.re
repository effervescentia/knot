open Knot.Kore;

let pp_operator: Fmt.t(AST.binary_t) =
  ppf =>
    (
      fun
      | AST.Equal => KEqual.Plugin.pp
      | AST.Unequal => KUnequal.Plugin.pp

      | AST.LogicalAnd => KLogicalAnd.Plugin.pp
      | AST.LogicalOr => KLogicalOr.Plugin.pp

      | AST.LessOrEqual => KLessOrEqual.Plugin.pp
      | AST.LessThan => KLessThan.Plugin.pp
      | AST.GreaterOrEqual => KGreaterOrEqual.Plugin.pp
      | AST.GreaterThan => KGreaterThan.Plugin.pp

      | AST.Add => KAdd.Plugin.pp
      | AST.Subtract => KSubtract.Plugin.pp
      | AST.Multiply => KMultiply.Plugin.pp
      | AST.Divide => KDivide.Plugin.pp

      | AST.Exponent => KExponentiate.Plugin.pp
    )
    % (pp => pp(ppf, ()));

let pp_binary_operation:
  Fmt.t(AST.raw_expression_t) =>
  Fmt.t((AST.binary_t, AST.expression_t, AST.expression_t)) =
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
