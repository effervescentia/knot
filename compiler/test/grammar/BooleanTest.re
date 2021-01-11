open Kore;
open Util;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    type t = Block.t(AST.primitive_t);

    let parser = Parser.parse(Primitive.boolean);

    let test =
      Alcotest.(
        check(
          testable(_ => fmt_block(fmt_prim) % Format.print_string, (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar - Boolean"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse true"
    >: (
      () =>
        ["true", " true "] |> Assert.parse_all(AST.of_bool(true) |> as_bool)
    ),
    "parse false"
    >: (
      () =>
        ["false", " false "]
        |> Assert.parse_all(AST.of_bool(false) |> as_bool)
    ),
  ];
