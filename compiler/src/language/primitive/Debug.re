open Knot.Kore;
open AST;

let to_xml =
  Primitive.(
    fun
    | Nil => KNil.Plugin.to_xml()
    | Boolean(x) => KBoolean.Plugin.to_xml(x)
    | Number(x) => KNumber.Plugin.to_xml(x)
    | String(x) => KString.Plugin.to_xml(x)
  );
