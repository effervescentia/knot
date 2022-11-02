open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;

let string = Matchers.string >|= Node.map(AST.Raw.of_string);
