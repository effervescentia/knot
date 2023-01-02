open Knot.Kore;
open Parse.Kore;

let parse =
  many1(Matchers.digit)
  >|= Input.join
  >|= Node.map(Int64.of_string)
  |> Matchers.lexeme;
