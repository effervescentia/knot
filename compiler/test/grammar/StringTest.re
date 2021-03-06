open Kore;
open Util;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    type t = Block.t(AST.primitive_t);

    let parser = Parser.parse(Primitive.string);

    let test =
      Alcotest.(
        check(
          testable(
            pp => fmt_block(fmt_prim) % Format.pp_print_string(pp),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar - String"
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
