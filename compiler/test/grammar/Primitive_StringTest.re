open Kore;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    type t = AST.primitive_t;

    let parser = Parser.parse(Primitive.string);

    let test =
      Alcotest.(
        check(
          testable(
            (x, y) => AST.print_prim(y) |> Format.print_string,
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Primitive - String"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse" >: (() => Assert.parse("\"foo\"", AST.of_string("foo"))),
    "with escape characters"
    >: (() => Assert.parse("\"foo\\\"bar\"", AST.of_string("foo\\\"bar"))),
  ];
