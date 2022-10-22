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
    | Number(num) => KNumber.Plugin.pp(ppf, num)
    | String(str) => KString.Plugin.pp(ppf, str);
