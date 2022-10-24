open Knot.Kore;
open Parse.Onyx;

let parse =
  choice([
    KNil.Plugin.parse,
    KBoolean.Plugin.parse,
    KNumber.Plugin.parse,
    KString.Plugin.parse,
  ]);

let pp: Fmt.t(AST.primitive_t) =
  ppf =>
    fun
    | Nil => KNil.Plugin.pp(ppf, ())
    | Boolean(x) => KBoolean.Plugin.pp(ppf, x)
    | Number(x) => KNumber.Plugin.pp(ppf, x)
    | String(x) => KString.Plugin.pp(ppf, x);
