open Knot.Kore;

let pp_operator: Fmt.t(AST.Operator.Binary.t) =
  ppf =>
    AST.Operator.Binary.(
      fun
      | Equal => KEqual.Plugin.pp
      | Unequal => KUnequal.Plugin.pp

      | LogicalAnd => KLogicalAnd.Plugin.pp
      | LogicalOr => KLogicalOr.Plugin.pp

      | LessOrEqual => KLessOrEqual.Plugin.pp
      | LessThan => KLessThan.Plugin.pp
      | GreaterOrEqual => KGreaterOrEqual.Plugin.pp
      | GreaterThan => KGreaterThan.Plugin.pp

      | Add => KAdd.Plugin.pp
      | Subtract => KSubtract.Plugin.pp
      | Multiply => KMultiply.Plugin.pp
      | Divide => KDivide.Plugin.pp

      | Exponent => KExponentiate.Plugin.pp
    )
    % (pp => pp(ppf, ()));

let pp_binary_operation:
  Fmt.t(AST.Result.raw_expression_t) =>
  Fmt.t(
    (AST.Operator.Binary.t, AST.Result.expression_t, AST.Result.expression_t),
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
