open Kore;
open Util.RawUtil;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    open AST;

    type t = Raw.primitive_t;

    let parser = _ => Parser.parse(Primitive.string);

    let test =
      Alcotest.(
        check(
          testable(pp => Raw.Dump.prim_to_entity % Dump.Entity.pp(pp), (==)),
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
        |> Assert.parse_all(AST.Raw.of_string("foo") |> as_raw_node)
    ),
    "with escape characters"
    >: (
      () =>
        Assert.parse(
          "\"foo\\\"bar\"",
          AST.Raw.of_string("foo\\\"bar") |> as_raw_node,
        )
    ),
  ];
