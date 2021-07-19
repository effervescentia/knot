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
      () =>
        ["\"foo\"", " \"foo\" "]
        |> Assert.parse_all("foo" |> RawUtil.string |> as_string)
    ),
    "with escape characters"
    >: (
      () =>
        Assert.parse(
          "\"foo\\\"bar\"",
          "foo\\\"bar" |> RawUtil.string |> as_string,
        )
    ),
  ];
