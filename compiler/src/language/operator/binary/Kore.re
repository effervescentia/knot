open AST.Framework.BinaryOperator;
open Knot.Constants;
open AST.Operator.Binary;

include Knot.Kore;

module KExponentiate = KExponentiate.Plugin;

module KEqual =
  Make({
    let operator = (Equal, Glyph(Glyph.equality));
  });

module KUnequal =
  Make({
    let operator = (Unequal, Glyph(Glyph.inequality));
  });

module KLogicalAnd =
  Make({
    let operator = (LogicalAnd, Glyph(Glyph.logical_and));
  });
module KLogicalOr =
  Make({
    let operator = (LogicalOr, Glyph(Glyph.logical_or));
  });

module KLessOrEqual =
  Make({
    let operator = (LessOrEqual, Glyph(Glyph.less_or_eql));
  });
module KLessThan =
  Make({
    let operator = (LessThan, Glyph(Glyph.less_than));
  });
module KGreaterOrEqual =
  Make({
    let operator = (GreaterOrEqual, Glyph(Glyph.greater_or_eql));
  });
module KGreaterThan =
  Make({
    let operator = (GreaterThan, Glyph(Glyph.greater_than));
  });

module KAdd =
  Make({
    let operator = (Add, Character(Character.plus_sign));
  });
module KSubtract =
  Make({
    let operator = (Subtract, Character(Character.minus_sign));
  });
module KMultiply =
  Make({
    let operator = (Multiply, Character(Character.asterisk));
  });
module KDivide =
  Make({
    let operator = (Divide, Character(Character.forward_slash));
  });
