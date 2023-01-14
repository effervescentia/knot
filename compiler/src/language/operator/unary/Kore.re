open Knot.Constants.Character;
open AST.Framework.UnaryOperator;
open AST.Operator.Unary;

include Knot.Kore;

module KLogicalNot =
  Make({
    let operator = (Not, exclamation_mark);
  });
module KAbsolute =
  Make({
    let operator = (Positive, plus_sign);
  });
module KNegative =
  Make({
    let operator = (Negative, minus_sign);
  });
