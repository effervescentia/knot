open Knot.Kore;
open Parse.Kore;
open AST;

let parse = parse_expr =>
  parse_expr
  |> Matchers.between_parentheses
  >|= (node => Node.untyped(Raw.of_group(fst(node)), Node.get_range(node)));
