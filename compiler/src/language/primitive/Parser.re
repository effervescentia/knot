open Knot.Kore;
open Parse.Kore;
open AST;

let parse_primitive =
  choice([
    KNil.Plugin.parse,
    KBoolean.Plugin.parse,
    KNumber.Plugin.parse,
    KString.Plugin.parse,
  ]);

let parse = () => parse_primitive >|= Node.map(Raw.of_prim);
