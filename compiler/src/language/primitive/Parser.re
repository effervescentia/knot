open Kore;
open Parse.Kore;

let (&>) = (parse, to_primitive) => parse >|= Node.map(to_primitive);

let parse_primitive =
  Interface.(
    choice([
      KNil.parse &> (_ => nil),
      KBoolean.parse &> of_boolean,
      KFloat.parse &> of_float,
      KInteger.parse &> of_integer,
      KString.parse &> of_string,
    ])
  );

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  (f, ()) => parse_primitive >|= Node.map(f);
