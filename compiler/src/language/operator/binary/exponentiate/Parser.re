open Knot.Kore;
open Parse.Kore;
open AST;

type parse_t('expr) =
  (
    ((AST.Operator.Binary.t, Common.raw_t('expr), Common.raw_t('expr))) =>
    'expr
  ) =>
  Parse.Parser.t(
    (Common.raw_t('expr), Common.raw_t('expr)) => Common.raw_t('expr),
  );

let parse: parse_t('expr) =
  f =>
    Tuple.fold2(
      Parse.Util.binary_op(((lhs, rhs)) =>
        (Operator.Binary.Exponent, lhs, rhs) |> f
      ),
    )
    <$ Matchers.symbol(Constants.Character.caret);
