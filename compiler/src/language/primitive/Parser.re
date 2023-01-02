open Kore;
open Parse.Kore;
open AST;

let parse_primitive =
  choice([
    KNil.parse,
    KBoolean.parse >|= Node.map(Raw.of_bool),
    KFloat.parse >|= Node.map(Raw.of_float),
    KInteger.parse >|= Node.map(Raw.of_int),
    KString.parse >|= Node.map(Raw.of_string),
  ]);

let parse = () => parse_primitive >|= Node.map(Raw.of_prim);
