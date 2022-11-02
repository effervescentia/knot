open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Type = AST.Type;

let parse: primitive_parser_t =
  choice([
    KNil.Plugin.parse,
    KBoolean.Plugin.parse,
    KNumber.Plugin.parse,
    KString.Plugin.parse,
  ]);

let analyze: AST.Result.primitive_t => Type.t =
  fun
  | Nil => Valid(`Nil)
  | Boolean(_) => Valid(`Boolean)
  | Number(Integer(_)) => Valid(`Integer)
  | Number(Float(_)) => Valid(`Float)
  | String(_) => Valid(`String);

let pp: Fmt.t(AST.Result.primitive_t) =
  ppf =>
    fun
    | Nil => KNil.Plugin.pp(ppf, ())
    | Boolean(x) => KBoolean.Plugin.pp(ppf, x)
    | Number(x) => KNumber.Plugin.pp(ppf, x)
    | String(x) => KString.Plugin.pp(ppf, x);
