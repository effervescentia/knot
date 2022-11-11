open Knot.Kore;
open AST;

module KBoolean = KBoolean.Plugin;
module KNil = KNil.Plugin;
module KNumber = KNumber.Plugin;
module KString = KString.Plugin;

let parse_primitive =
  Parse.Kore.(
    choice([KNil.parse, KBoolean.parse, KNumber.parse, KString.parse])
  );

let pp =
  Primitive.(
    ppf =>
      fun
      | Nil => KNil.format(ppf, ())
      | Boolean(x) => KBoolean.format(ppf, x)
      | Number(x) => KNumber.format(ppf, x)
      | String(x) => KString.format(ppf, x)
  );

let analyze =
  Primitive.(
    Type.(
      fun
      | Nil => Valid(`Nil)
      | Boolean(_) => Valid(`Boolean)
      | Number(Integer(_)) => Valid(`Integer)
      | Number(Float(_)) => Valid(`Float)
      | String(_) => Valid(`String)
    )
  );

include Framework.Expression({
  type parse_arg_t = unit;

  type value_t('a) = Primitive.primitive_t;

  let parse = () => Parse.Kore.(parse_primitive >|= Node.map(Raw.of_prim));

  let format = _ => pp;

  let to_xml = _ =>
    Primitive.(
      fun
      | Nil => KNil.to_xml()
      | Boolean(x) => KBoolean.to_xml(x)
      | Number(x) => KNumber.to_xml(x)
      | String(x) => KString.to_xml(x)
    );
});
