open Kore;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    type t = AST.primitive_t;

    let parser = Parser.parse(Primitive.boolean);

    let test =
      Alcotest.(
        check(
          testable(_ => AST.print_prim % Format.print_string, (==)),
          "program matches",
        )
      );
  });

let suite =
  "Primitive - Boolean"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse true" >: (() => Assert.parse("true", AST.of_bool(true))),
    "parse false" >: (() => Assert.parse("false", AST.of_bool(false))),
  ];
