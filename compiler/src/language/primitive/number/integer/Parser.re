open Knot.Kore;
open Parse.Kore;
open AST;

let integer =
  many1(Matchers.digit)
  >|= Input.join
  >|= Node.map(Int64.of_string % Raw.of_int)
  |> Matchers.lexeme;
