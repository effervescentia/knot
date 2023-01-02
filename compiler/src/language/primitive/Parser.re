open Kore;
open Parse.Kore;
open AST;

let (&>) = (parse, to_primitive) => parse >|= Node.map(to_primitive);

let parse_primitive =
  choice([
    KNil.parse &> (_ => Raw.nil),
    KBoolean.parse &> Raw.of_bool,
    KFloat.parse &> Raw.of_float,
    KInteger.parse &> Raw.of_int,
    KString.parse &> Raw.of_string,
  ]);

let parse = () => parse_primitive >|= Node.map(Raw.of_prim);
