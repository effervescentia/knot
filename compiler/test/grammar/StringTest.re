open Kore;
open AST.Raw.Util;
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
              % AST.Raw.Debug.print_prim
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
      () => ["\"foo\"", " \"foo\" "] |> Assert.parse_all(raw_string("foo"))
    ),
    "with escape characters"
    >: (() => Assert.parse("\"foo\\\"bar\"", raw_string("foo\\\"bar"))),
  ];
