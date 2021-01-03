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
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse"
    >: (
      () =>
        ["\"foo\"", " \"foo\" "] |> Assert.parse_all(AST.of_string("foo"))
    ),
    "with escape characters"
    >: (() => Assert.parse("\"foo\\\"bar\"", AST.of_string("foo\\\"bar"))),
  ];
