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
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse true"
    >: (() => ["true", " true "] |> Assert.parse_all(AST.of_bool(true))),
    "parse false"
    >: (() => ["false", " false "] |> Assert.parse_all(AST.of_bool(false))),
  ];
