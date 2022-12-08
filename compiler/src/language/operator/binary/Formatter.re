open Knot.Kore;
open AST;

let format_operator: Fmt.t(Operator.Binary.t) =
  ppf =>
    Operator.Binary.(
      fun
      | Equal => KEqual.Plugin.format
      | Unequal => KUnequal.Plugin.format

      | LogicalAnd => KLogicalAnd.Plugin.format
      | LogicalOr => KLogicalOr.Plugin.format

      | LessOrEqual => KLessOrEqual.Plugin.format
      | LessThan => KLessThan.Plugin.format
      | GreaterOrEqual => KGreaterOrEqual.Plugin.format
      | GreaterThan => KGreaterThan.Plugin.format

      | Add => KAdd.Plugin.format
      | Subtract => KSubtract.Plugin.format
      | Multiply => KMultiply.Plugin.format
      | Divide => KDivide.Plugin.format

      | Exponent => KExponentiate.Plugin.format
    )
    % (pp => pp(ppf, ()));

let format:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Operator.Binary.t, Result.expression_t, Result.expression_t)) =
  (pp_expression, ppf, (op, (lhs, _), (rhs, _))) =>
    Fmt.pf(
      ppf,
      "%a %a %a",
      pp_expression,
      lhs,
      format_operator,
      op,
      pp_expression,
      rhs,
    );
