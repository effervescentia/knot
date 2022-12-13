open Kore;
open AST;

let format_operator: Fmt.t(Operator.Binary.t) =
  ppf =>
    Operator.Binary.(
      fun
      | Equal => KEqual.format
      | Unequal => KUnequal.format

      | LogicalAnd => KLogicalAnd.format
      | LogicalOr => KLogicalOr.format

      | LessOrEqual => KLessOrEqual.format
      | LessThan => KLessThan.format
      | GreaterOrEqual => KGreaterOrEqual.format
      | GreaterThan => KGreaterThan.format

      | Add => KAdd.format
      | Subtract => KSubtract.format
      | Multiply => KMultiply.format
      | Divide => KDivide.format

      | Exponent => KExponentiate.format
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
