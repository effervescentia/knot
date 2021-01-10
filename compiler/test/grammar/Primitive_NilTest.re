open Kore;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    type t = AST.primitive_t;

    let parser = Parser.parse(Primitive.nil);

    let test =
      Alcotest.(
        check(
          testable(_ => fmt_prim % Format.print_string, (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar - Primitive - Nil"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse" >: (() => ["nil", " nil "] |> Assert.parse_all(AST.nil)),
  ];
