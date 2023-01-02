open Knot.Kore;
open Parse.Kore;
open AST;

let parse = parse_expression =>
  parse_expression
  |> Matchers.between_parentheses
  >|= Node.wrap(fst % Raw.of_group);
