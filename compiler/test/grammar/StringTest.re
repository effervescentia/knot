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
          testable(
            pp =>
              Node.Raw.value
              % Raw.Debug.print_prim
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
        |> Assert.parse_all(AST.Raw.of_string("foo") |> as_lexeme)
    ),
    "with escape characters"
    >: (
      () =>
        Assert.parse(
          "\"foo\\\"bar\"",
          AST.Raw.of_string("foo\\\"bar") |> as_lexeme,
        )
    ),
  ];
