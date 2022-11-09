open Knot.Kore;
open Parse.Kore;

let string = Matchers.string >|= Node.map(AST.Raw.of_string);
