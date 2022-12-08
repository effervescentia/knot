open Kore;
open Parse.Kore;
open AST;

let parse_primitive =
  choice([KNil.parse, KBoolean.parse, KNumber.parse, KString.parse]);

let parse = () => parse_primitive >|= Node.map(Raw.of_prim);
