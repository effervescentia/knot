open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Boolean = KBoolean.Plugin;
module Nil = KNil.Plugin;
module Number = KNumber.Plugin;
module String = KString.Plugin;
module Type = AST.Type;

type t = AST.Primitive.primitive_t;

let parse: primitive_parser_t =
  choice([Nil.parse, Boolean.parse, Number.parse, String.parse]);

let analyze: t => Type.t =
  fun
  | Nil => Valid(`Nil)
  | Boolean(_) => Valid(`Boolean)
  | Number(Integer(_)) => Valid(`Integer)
  | Number(Float(_)) => Valid(`Float)
  | String(_) => Valid(`String);

let pp: Fmt.t(t) =
  ppf =>
    fun
    | Nil => Nil.pp(ppf, ())
    | Boolean(x) => Boolean.pp(ppf, x)
    | Number(x) => Number.pp(ppf, x)
    | String(x) => String.pp(ppf, x);

let to_xml: t => Fmt.xml_t(string) =
  fun
  | Nil => Nil.to_xml()
  | Boolean(x) => Boolean.to_xml(x)
  | Number(x) => Number.to_xml(x)
  | String(x) => String.to_xml(x);
