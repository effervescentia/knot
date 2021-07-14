open Kore;
open Util;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    type t = AST.primitive_t;

    let parser = _ => Parser.parse(Primitive.string);

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Tuple.fst3
              % Debug.print_prim
              % Pretty.to_string
              % Format.pp_print_string(pp),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Primitive (String)"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse"
    >: (
      () =>
        ["\"foo\"", " \"foo\" "]
        |> Assert.parse_all(AST.of_string("foo") |> as_string)
    ),
    "with escape characters"
    >: (
      () =>
        Assert.parse(
          "\"foo\\\"bar\"",
          AST.of_string("foo\\\"bar") |> as_string,
        )
    ),
  ];
