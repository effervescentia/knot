open Knot.Kore;
open Parse.Kore;
open AST;

let parse:
  Parse.Parser.t((Raw.expression_t, Raw.expression_t) => Raw.expression_t) =
  Tuple.fold2(Parse.Util.binary_op(Raw.of_expo_op))
  <$ Matchers.symbol(Constants.Character.caret);
